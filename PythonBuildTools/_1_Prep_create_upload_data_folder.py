# Filename: 0_Prep_create_upload_data_folder.py
# Goal: 
#   1. compress and copy files from data_uncompressed to the data directory
# Created: 20200114
# Author: Chris Armbrust
# Revision: 20200114

try:
    import configparser
except ImportError:
    import ConfigParser as configparser




import os
import gzip
import shutil
import glob

import _0_config_environment as configs

def prepare_www_files(source, target, env):
    #WARNING -  this script will DELETE the 'data' dir and recreate an empty one to copy/gzip files from 'data_src'
    #           so make sure to edit the files in 'data_src' folder as any changes made to files in 'data' woll be LOST
    #           
    #           If 'data_src' dir doesn't exist, and 'data' dir is found, the script will automatically
    #           rename 'data' to 'data_src
    print('[COPY/GZIP DATA FILES]')
    result = 0 # if not complete this is still 0

    #add filetypes (extensions only) to be gzipped before uploading. 
    filetypes_to_gzip = ['min.js', 'htm', 'min.css', 'mode-html.js', 'tnb.js', 'mode-json.js', 'worker-json.js','worker-css.js', 'worker-html.js']
    filetypes_to_exclude = ['js', 'html', 'css', 'txt', 'md','db','bat', 'htm', 'jpg', 'bak'] # all of file extensions to not copy
    # files_to_just_copy = ['mode-html.js', 'theme-tomorrow_night_bright.js', 'mode-json.js', 'worker-json.js','worker-css.js', 'worker-html.js' ]
    files_to_just_copy = []
    files_to_gzip = [] 
    files_to_copy = [] 

    data_dir = os.path.join(env.get('PROJECT_DIR'), env.get('PROJECTDATA_DIR'))
    data_src_dir = os.path.join(env.get('PROJECT_DIR'), 'data_uncompressed')

    if(os.path.exists(data_dir) and not os.path.exists(data_src_dir) ):
        print('  "data" dir exists, "data_src" not found.')
        print('  renaming "' + data_dir + '" to "' + data_src_dir + '"')
        os.rename(data_dir, data_src_dir)

    if(os.path.exists(data_dir)):
        print('  Deleting data dir ' + data_dir)
        try: 
            shutil.rmtree(data_dir)
        except OSError as e:
            print("Failed to delete ", data_dir, " << okay to?") 

    print('  Re-creating empty data dir "' + data_dir + '" in project directory: "'+ env.get('PROJECT_DIR') +'"')
    os.chdir(env.get('PROJECT_DIR'))
    try:
        os.mkdir(data_dir)
    except OSError as e:
        print ("Trying to create data_dir directory: "+os.path.dirname(data_dir)+" returns error:", e)  
    os.chdir(data_src_dir)

        # files_to_gzip = [] #//TODO: need to walk thru the subdirectories also
        # for extension in filetypes_to_gzip:
        #     files_to_gzip.extend(glob.glob(os.path.join(data_src_dir, '*.' + extension)))
        
        # print('  files to gzip: ' + str(files_to_gzip))

        # # //TODO: add files to exclude from copy 


        # all_files = glob.glob(os.path.join(data_src_dir, '*.*'))
        # files_to_copy = list(set(all_files) - set(files_to_gzip))
    # os.chdir(source)
    for root, dirs, files in os.walk("."):
        print ("Directories: ")
        for directory in dirs:
            print (os.path.join(root,directory))
        print ("Files: ")
        for filename in files:
            full_filename= os.path.join(root,filename)
            print(full_filename, len(full_filename))
            for ext in filetypes_to_gzip:
                if full_filename.endswith(ext):
                    # print (gzip ext, ">>>", filename)
                    files_to_gzip.append(full_filename)
            for ext in files_to_just_copy:
                if full_filename.endswith(ext):
                    # print (just copy ext, ">>>", filename)
                    files_to_copy.append(full_filename)                            
            exclude_filename=False
            for ext in filetypes_to_exclude:
                do_not_copy = full_filename.endswith(ext)
                # print (okay_to_copy, ext, full_filename)
                if (do_not_copy== True):
                    exclude_filename=True
                    break

                    # print (ext, ">>>", filename)
            # print ("if False - okay to copy:", exclude_filename, ext, full_filename)   
            if (exclude_filename==False): 
                files_to_copy.append(full_filename)           

    print('  files to copy: ' + str(files_to_copy))
    print('  files to gzip: ' + str(files_to_gzip))

    for file in files_to_copy:
        full_filename= os.path.normpath(os.path.join(data_dir, file))
        print('  Copying file: ' + file +  ' to data dir '+ full_filename)
        # makedir if directory doesn't already exist
        try:
            os.makedirs(os.path.dirname(full_filename))  
        except OSError as e:
            # print ("creating Directory: "+os.path.dirname(full_filename)+" returns error:", e)
            if e.errno != 17:
                raise
        shutil.copy(file, full_filename)
    
    print('  GZipping files: ')
        # NOTE:  you CAN'T use the '.gz' extension when serving compressed CSS or JS files to Safari. So, using '.jgz' as extension
    for file in files_to_gzip:
        full_filename= os.path.normpath(os.path.join(data_dir, file))
        
        # makedir if directory doesn't already exist
        try:
            os.makedirs(os.path.dirname(full_filename))  
        except OSError as e:
            # print ("creating Directory: "+os.path.dirname(full_filename)+" returns error:", e)
            # print (e.errno)
            if e.errno != 17:
                print ("creating Directory: "+os.path.dirname(full_filename)+" returns error:", e)
                raise
        # with open(file, 'rb') as src, gzip.open(os.path.join(full_filename), 'wb') as dst:  
            # is .gz extention then can't use editor. So, files with .htm, .js and .css are assumed to be compressed. Handled by ESP Async Webserver
        with open(file, 'rb') as src, gzip.open(os.path.join(full_filename+ '.gz'), 'wb') as dst:              
                shutil.copyfileobj(src,dst)
                print(full_filename)
                # dst.writelines(src)

    print('[/COPY/GZIP DATA FILES]')
    result = 1 # success
    return result


def create_data_dir(source, target, env):
    
    print("Begin prep of files.... ")
    if (prepare_www_files(source,target, env)):
        print ("Gzip and Copy of 'data' directory complete!")
        os.chdir(env.get('PROJECT_DIR'))
        try:
            print("Begin create spiffs.bin")
            # create_SPIFFS_Bin()
            create_SPIFFS_bin_pio()
            # subprocess.check_call("pio run --target uploadfs")
            print ("spiffs.bin created from create_data_dir.")
        except Exception as e:
        # except subprocess.CalledProcessError:
            print("Error: ", e, "\n\t creating the directory: ", source,"\n\t with target: ", target, "\n\tand env:" ,env)

# def extras_create_data_dir(): # DEPRECATED
#     print("'"+ __name__ + "' called from Upload_Data_and_Bins.py" ) # otherwise the upload happens before the file prep
#     local_root_dblBack='C:\\Users\\carmb\\CloudStation\\Arduino\\User_Sketches\\PaGoI-MILwright'
#     source = local_root_dblBack+"\\data_uncompressed"
#     target = local_root_dblBack+"\\data"
#     env = {'PROJECTDATA_DIR':'data','PROJECT_DIR':local_root_dblBack }
#     create_data_dir(source, target, env)

def create_SPIFFS_bin_pio():
    # pio.exe run --target buildfs --environment d1_mini
    import subprocess
    subprocess.run('c:\\Users\\carmb\\.platformio\\penv\\Scripts\\platformio.exe run --target buildfs --environment d1_mini')

def create_SPIFFS_Bin():
    # \pio.exe run --target uploadfs --environment d1_mini
    import subprocess
    mkspiffs_exe = "c:\\Users\\carmb\\.platformio\\packages\\tool-mkspiffs\\mkspiffs_espressif8266_arduino.exe"
    subprocess.run(mkspiffs_exe + " -c " + env.get('PROJECTDATA_DIR') + " -p 256 -b 4096 -s 1024000 .pio\\build\\d1_mini\\spiffs.bin")
    # subprocess.run(mkspiffs_exe + " -c " + env.get('PROJECTDATA_DIR') + " -p 256 -b 4096 -s 0x100000 .pio\\build\\d1_mini\\spiffs.bin")
    Littlefs_exe = env.get('PROJECT_DIR') + "\\PythonBuildTools\\mklittlefs.exe -c "
    subprocess.run(Littlefs_exe + env.get('PROJECTDATA_DIR') + " -p 256 -b 4096 -s 1024000 .pio\\build\\d1_mini\\Littlefs.bin")
if __name__ != '__main__':
    # Import("env")
    print ("Loading Pre build scripts") 
    # local_root_dblBack='C:\\Users\\carmb\\CloudStation\\Arduino\\User_Sketches\\PaGoI-MILwright'
    source = configs.local_root_dblBack+"\\data_uncompressed"
    target = configs.local_root_dblBack+"\\data"
    env = {'PROJECTDATA_DIR':'data','PROJECT_DIR':configs.local_root_dblBack + configs.project_dir }
    create_data_dir(source, target, env)
    # # if not running from main - assume running from Platformio as part of build so only get the data ready in a SPIFFS
    # if (prepare_www_files(source,target, env)):
    #     print ("Gzip and Copy of 'data' directory complete!")


    # # Import("env", "projenv") # note: projenv is available only for POST-type scripts
    # # env.AddPostAction("buildprog", after_build)
    # # first parm should be target, e.g.: buildfs
    # print("Adding pre-action python function 'prepare_www_files' for '$BUILD_DIR/spiffs.bin'")
    # data_dir = os.path.join(env.get('PROJECT_DIR'), env.get('PROJECTDATA_DIR'))
    # # print("Removing the existing directory: ", data_dir)
    # # if (os.path.exists(data_dir)):
    # #     shutil.rmtree(data_dir)
    # env.AddPreAction('$PROJECT_DIR', prepare_www_files)
    # # env.AddPreAction('uploadfs', prepare_www_files)
    # print("AddPreAction added. ")

if __name__ == '__main__':
    import subprocess
    print("'"+ __name__ + "' called from Upload_Data_and_Bins.py") # otherwise the upload happens before the file prep
        
    source = configs.local_root_dblBack+"\\data_uncompressed"
    target = configs.local_root_dblBack+"\\data"
    env = {'PROJECTDATA_DIR':'data','PROJECT_DIR':configs.local_root_dblBack + configs.project_dir }

    create_data_dir(source, target, env)
    # if (prepare_www_files(source,target, env)):
    #     print ("Gzip and Copy of 'data' directory complete!")


    # import os
    # os.system('c:\\Users\\carmb\\AppData\\Local\\Programs\\Python\\Python38\\python.exe ' + currentdir +'\AfterBuild_Copy_bins_to_AWS_Server.py')

