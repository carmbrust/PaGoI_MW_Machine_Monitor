# Filename: Copy_bin_to_Server_ssh.py
# Goal: Using the Python library Paramiko to implement a SSH client:
#       1. programmatically connect to another computer over SSH and 
#       2. copy binaries to the server. 

# Created: 20190826
# Author: Chris Armbrust
# Revision: 20200727
# Execute Shell Commands Over SSH Using Python and Paramiko
# from __future__ import print_function


import os
import base64
try: 
        import paramiko
except ImportError:
        os.system('c:\\Users\\carmb\\AppData\\Local\\Programs\\Python\\Python38\\python.exe -m pip install paramiko')
        # env.Execute("$PYTHONEXE -m pip install paramiko")
        import paramiko

#import re
import datetime

import _0_config_environment as configs
#from AfterBuild_Copy_bins_to_AWS_Server import get_build_flag_value


def sftp_put_preserve_timestamp(sftp_connection, localpath, remotepath):
    local_stat = os.stat(localpath)
    times = (local_stat.st_atime, local_stat.st_mtime)
    sftp_connection.put(localpath, remotepath)
    sftp_connection.utime(remotepath, times)
    mod_timestamp = datetime.datetime.fromtimestamp(local_stat.st_mtime)
    print ("\tto:", remotepath, " \tlast modified: ", mod_timestamp)


def open_ssh_connection(server, port, username, keyfile, keyfile_type):
        #open_ssh_connection(server, port, username, keyfile_path, keyfile_type)
        print ("Begin open ssh connection: \n\tServer: ", server, "\n\tusername:", username, "\n\tprivate keyfile:", keyfile, "\n\tkeyfile_type:", keyfile_type)
        ssh = paramiko.SSHClient()


        # cert = paramiko.RSAKey.from_private_key_file(keyfile)
        # # cert = paramiko.RSAKey.from_private_key_file('u:\AWS\AWS_EC2_172.pem')
        # # print ("cert = '", cert, "'")
        # #  host = 'ec2-52-36-125-172.us-west-2.compute.amazonaws.com'
        # #     port = 22
        # #     username = 'ubuntu'
        # #     password = 'Arrgh2016!'
        # #     # keyfile_path = 'private_key_file'
        # #     keyfile_path = '\\diskstation\\home\AWS\\EC2_Key_pair.pem'
        # #     keyfile_type = 'RSA'

        # ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        # # ssh.load_host_keys(os.path.expanduser(os.path.join("~", ".ssh", "known_hosts")))
        # # ssh.connect(server, username=username, password=password, pkey = cert)
        # ssh.connect(server, username=username,  pkey = cert)
        key = paramiko.RSAKey(data=base64.b64decode(b'AAAAB3NzaC1yc2EAAAADAQABAAABAQC6bqYjY1h+0wbN3Xc2Y5k44QUDSpTOhCfuB1a0l2JCRwvKrlt9FpadVDHmK0fOAsmer0hgosTyI0NR7t+ulkK59Xd7+0PHPd15ZLRE5LdHxR5XdkqXaf4UM2R4IT41lDnBBM1siBZddt9TP0t1e6+2inIgv02VvUeS801W/VllTFE7Ud6vgHpbvxc1kUeqVXbJWo8A3VFnqnMcXNQUA6ta3O9623tNm2VkgniRnmvtdwJ7/DKUe+ov2sQ6uIyKUt85MtdjswwyUl3zhVQ4UzqJ6ozGtBIRTrvp5iTkzi802KhwzPI0ZsDJ4sKoPqHdLFeXJGLlU0fxNo3P24tkY9fz'))
        ssh.get_host_keys().add(server, 'ssh-rsa', key)
        ssh.connect(server, username=username, password=password)

        print ("Connected to: ", server)
        # ssh.connect(server, username=username, password=password, pkey=keyfile_path)
        # ssh.connect(server, username=username, pkey = cert)
        return ssh

def sftp_copy(ssh, localPath, remotePath, fileType):
        sftp = ssh.open_sftp()
        print("Source directory: ", localPath)
        filelist = os.listdir( localPath )
        for filename in filelist:
                if  (filename.endswith(fileType)):
                        print("Copying file:" , filename)
                        # print("Copying file:" , filename, end='' )
                        sftp_put_preserve_timestamp(sftp, localPath + "/" + filename, remotePath + "/" + filename)
        sftp.close()

  
if __name__ != '__main__':
    
#     Import("env", "projenv")
    print(os.path.basename(__file__) + " running as not __main___")

    # keep pip at the latest
    # env.Execute('$PYTHONEXE  -m pip install --upgrade pip')
    

#     # access to global build environment
#     print("env:':",env,"'")

#     # access to project build environment (is used source files in "src" folder)
#     print("projenv:':",projenv,"'")

def get_build_flag_value(flag_name):
#     build_flags = env.ParseFlags(env['BUILD_FLAGS'])
#     flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
#     defines = {k: v for (k, v) in flags_with_value_list}
## //HACK: force return of project name
        flag_name="MILwright"
        return flag_name
        # return defines.get(flag_name)

port = 22

if get_build_flag_value('PAGOI_CLIENT') == "PAGOI":
        server = "demo.pagoi.com"
        username ="ubuntu"
elif get_build_flag_value('PAGOI_CLIENT') == "MILwright":
        # RPi 
        server = "rpi3-md.local"
        username = "pi"
        password ="MILwright!"
        # # AWS
        # server = "milwright.pagoi.com"
        # username ="ec2-user"
elif get_build_flag_value('PAGOI_CLIENT') == "Arrgh":
        # RPi 
        server = "192.168.1.128"
        username = "pi"
        password ="Arrgh2016!"
else: 
        server = "demo.pagoi.com"
        username ="ubuntu"


# private key location: \\diskstation\home\CloudStation\AWS\AWS_EC2_172.ppk
# 
# for MILwright
# \\diskstation\home\CloudStation\AWS\EC2_172.ppk

keyfile_path = configs.local_root_dblBack+'\\AWS\\aws_mw_2020_ssh.pem'
keyfile_path = configs.local_root_dblBack+ configs.project_dir + '\\PythonBuildTools\\rpi3-md_ssh_host_rsa_key.pub'
keyfile_path = configs.sshPrivateKey
keyfile_type = 'ssh-rsa'

if __name__ == '__main__':
        print ("Running as __main__", os.path.basename(__file__))  
        ssh_connect = open_ssh_connection(server, port, username, keyfile_path, keyfile_type)
        # # try a shell command
        # stdin,stdout,stderr = ssh_connect.exec_command("ls -al")
        # # process results
        # for line in stdout.readlines():
        #         print (line.strip())
        local_path = r'C:\Users\carmb\CloudStation\Clients\MILwright\Machine_Monitor\Builds'
        remote_path = "/var/www/html/OTA_Images/"
        file_type= ".bin"
        print ("local: ", local_path)
        print ("remote: ", remote_path)
        sftp_copy(ssh_connect, local_path, remote_path, file_type)

        ssh_connect.close()
        print ("ssh upload Complete!")

