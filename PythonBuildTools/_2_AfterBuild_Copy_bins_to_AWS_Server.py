# Filename: _2_AfterBuild_Copy_bins_to_AWS_Server.py
# Goal: look for new binary files
#       0. compress and copy files from 'data_uncompressed' to the 'data' directory (Prep_create_upload_data_folder)
#       1. copy with rename to the builds directory
#       2. ssh copy to AWS server
#       3. //TODO increment the current revision counter in the list of nodes
#       4. //TODO create/overwrite the MAC address numbered files with the
#           current desired revision for that nodes MAC
#       
# Created: 20190823
# Author: Chris Armbrust
# Revision: 20200727

import _0_config_environment as configs
import _3_Copy_bin_to_Server_ssh as ssh_copy
import fnmatch
import os
import shutil

try:    
    import configparser
except ImportError:
    os.system('python -m pip install configparser')
    try: 
        import configparser
    except ImportError:    
        import ConfigParser as configparser


print("Loading Post build scripts")

# config = configparser.ConfigParser()
# config.read("platformio.ini")
# host = config.get("env_custom_target", "custom_ping_host")

# def mytarget_callback(*args, **kwargs):
#     print("Hello PlatformIO!")
#     env.Execute("ping " + host)


# env.AlwaysBuild(env.Alias("ping", None, mytarget_callback))
def get_build_flag_value(flag_name):
    flag_value=""
    try:
        build_flags = env.ParseFlags(env['BUILD_FLAGS'])
        flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
        defines = {k: v for (k, v) in flags_with_value_list}
        flag_value= defines.get(flag_name)
        return flag_value
    except Exception:
        print("Exception: ", Exception)

    return flag_value

    
# from Copy_bin_to_AWS_Server import local_root_dblBack, local_root_sglBack

def CopyBins():
    print("Begin copy .bin files to build and OTA directories for: " + ssh_copy.get_build_flag_value('PAGOI_CLIENT'))
    print("Application version is: "+ App_Version)
    # TODO append (or prepend?) the APP_VERSION to the end of the bin filename before the '.bin'
    source = configs.local_root_dblBack + \
        "\\Arduino\\User_Sketches\\PaGoI-MILwright\\.pio\\build"
    dest1 = configs.local_root_dblBack+'\\clients\\PaGoI\\OTA_Images'
    
    
    # dest2 is the MD CloudStation, dest3 is on remote server (e.g. AWS)
    if ssh_copy.get_build_flag_value('PAGOI_CLIENT') == "PAGOI":
        dest1 = configs.local_root_dblBack+'\\clients\\PaGoI\\OTA_Images'
        dest2 = configs.local_root_dblBack + \
            '\\Clients\\PaGoI\\website_demo\\demo\\html\\OTA_Images'
        dest3 = "/var/www/demo/html/OTA_Images/"
    elif ssh_copy.get_build_flag_value('PAGOI_CLIENT') == "MILwright":
        dest1 = configs.local_root_dblBack+'\\Clients\\MILwright\\Machine_Monitor\\Builds'
        dest2 = configs.local_root_dblBack + \
            '\\Clients\\MILwright\\Machine_Monitor\\MM_Website\\website_deployed_20200512 - current\\OTA_Images'
        dest3 = "/var/www/html/OTA_Images/"
    elif ssh_copy.get_build_flag_value('PAGOI_CLIENT') == "Arrgh":
        dest2 = configs.local_root_dblBack + \
            '\\Clients\\Arrgh\\Server_AWS\\var-www\\html\\OTA_Images'
        dest3 = "/var/www/html/OTA_Images/"
    else: 
        dest2 = configs.local_root_dblBack + \
            '\\Clients\\PaGoI\\website_demo\\demo\\html\\OTA_Images'
        dest3 = "/var/www/demo/html/OTA_Images/"

    os.chdir(source)

    for root, dirs, files in os.walk("."):
        # print ("root: ", root, "directory: ", dirs)
        pattern = "*.bin"
        # files of interest are either firmware.bin or spiffs.bin
        for filename in files:
            if fnmatch.fnmatch(filename, pattern):
                dest_filename = "\\"+ssh_copy.get_build_flag_value('PAGOI_CLIENT') +"_"+ root[2:]+"_"+App_Version +"_"+ filename
                # Example: MILwright_d1_mini_21022_firmware.bin
                print(root+"\\"+filename + " "+dest1 + dest_filename)
                shutil.copy2(root+"\\"+filename, dest1 + dest_filename)
                print(root+"\\"+filename+" "+dest2 +  dest_filename)
                shutil.copy2(root+"\\"+filename, dest2  + dest_filename)

    # uncomment to upload to AWS
    print("Begin copy .bin files from builds directory (",
          dest1, ") \n\r to Update Webserver OTA_Images directory (", dest3, ")")

    # # AWS configuration info
    # server = "milwright.pagoi.com"
    # username ="ec2-user"
    # # server = "arrgh.pagoi.com"
    # port = 22
    # username ="ubuntu"
    # keyfile_path = 'u:\AWS\AWS_EC2_172.pem'
    # keyfile_type = 'RSA'

    # ssh_connect = ssh_copy.open_ssh_connection(server, port, username, keyfile_path, keyfile_type)
    ssh_connect = ssh_copy.open_ssh_connection(
        ssh_copy.server, ssh_copy.port, ssh_copy.username, ssh_copy.keyfile_path, ssh_copy.keyfile_type)

    local_path = dest1
    file_type = ".bin"

    ssh_copy.sftp_copy(ssh_connect, local_path, dest3, file_type)

    ssh_connect.close()
    # end of AWS copy block

def updateESP(ip_base,App_Version):
    ssh_connect = ssh_copy.open_ssh_connection(ssh_copy.server, ssh_copy.port, ssh_copy.username, ssh_copy.keyfile_path, ssh_copy.keyfile_type)
    # sh /var/www/html/OTA_Images/ESP_Network_Update.sh 136 MILwright_d1_mini_21022_firmware.bin MILwright_d1_mini_21022_spiffs.bin
    shell_command = 'sh /var/www/html/OTA_Images/ESP_Network_Update_over_SSH.sh ' + ip_base + ' MILwright_d1_mini_' + App_Version + '_firmware.bin MILwright_d1_mini_' + App_Version + '_spiffs.bin '
    print("Updating ESP: ", shell_command)
    stdin, stdout, stderr = ssh_connect.exec_command(shell_command)
    for line in stdout:
        print('... ' + line.strip('\n'))
    ssh_connect.close()


def after_build(source, target, env):
    print("Build complete - now copy all .bin files for: ", source, target, env)
    # do some actions
    # pio run -e d1_mini -e esp01_1m_160 -t buildfs
    os.chdir(env.get('PROJECT_DIR'))
    try:
        print("Begin running upload... ")
        # may want to change this to run 
        #  "mkspiffs" -c data -p 256 -b 8192 -s -2121728 .pio\build\d1_mini_pro\spiffs.bin <<< adjust filesize and destination directory
        subprocess.check_call("pio run --target uploadfs")
        print ("Upload complete from create_data_dir.")
    except CalledProcessError:
        print("Something went wrong with filesystem upload - check output above.")

def after_build_copyBins():
    CopyBins()
    print("Copy of file(s) complete!")


if __name__ != '__main__':
    print(__name__ + " not running as __main__")
    # if __name__ == 'Upload_Data_and_Bins':
    #     Import("env")
    #     env.AddPostAction("buildprog", after_build)
    # else:
    App_Version = get_build_flag_value("APP_VERSION")
    if App_Version=="":
        App_Version= configs.defaultAppVersion
    CopyBins()
    ip_base ="136"
    updateESP(ip_base, App_Version)
    print ("Binaries updated to all locations and ESP Updated!!")



if __name__ == '__main__':
    print(__name__ + " running as __main__")
    App_Version= configs.defaultAppVersion
    CopyBins()
    ip_base ="136"
    updateESP(ip_base, App_Version)
    print("Copy of .bin file(s) complete!")
