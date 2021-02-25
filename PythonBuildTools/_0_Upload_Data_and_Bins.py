#filename: Upload_Data_and_Bins.py
print(__name__)

#
# Dump build environment (for debug purpose)
# print(env.Dump())
#

#
# Change build flags in runtime
#
# env.ProcessUnFlags("-DVECT_TAB_ADDR")
# env.Append(CPPDEFINES=("VECT_TAB_ADDR", 0x123456789))

#
# Upload actions
#
def create_build ():
    import subprocess
    subprocess.run('c:\\Users\\carmb\\.platformio\\penv\\Scripts\\platformio.exe run --environment d1_mini')
    
def before_upload(source, target, env):
    print("before_upload")
    # Prepare file system
    import os,sys,inspect
    currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
    print(__name__ + " Current Directory: ", currentdir)
    parentdir = os.path.dirname(currentdir)
    print(__name__ + " Parent Directory: ",parentdir)
    # sys.path.insert(1, os.path.join(sys.path[0], '..'))
    # print ("sys.path: ",sys.path)
    # import Prep_create_upload_data_folder
    # Prep_create_upload_data_folder.extras_create_data_dir()
    # this instance will be run as __main__
    os.system('python '+currentdir+'\Prep_create_upload_data_folder.py')
    # call Node.JS or other script
    # env.Execute("node --version")

# this is defined in Prep_create_upload_data_folder.py, so shouldn't need to repeat here. 
# def create_SPIFFS_Bin():
#     import subprocess
#     mkspiffs_exe = "c:\\Users\\carmb\\.platformio\\packages\\tool-mkspiffs\\mkspiffs_espressif8266_arduino.exe"
#     subprocess.run(mkspiffs_exe + " -c " + env.get('PROJECTDATA_DIR') + " -p 256 -b 4096 -s 0x100000 .pio\\build\\d1_mini\\spiffs.bin")
  
def copy_Bins(source, target, env):
    import os,sys,inspect,subprocess
    print("copy_Bins function")
    currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
    print("currentdir: "+currentdir)
    # subprocess.run('python ' + currentdir +'\\AfterBuild_Copy_bins_to_AWS_Server.py')
    # subprocess.run('c:\\Users\\carmb\\AppData\\Local\\Programs\\Python\\Python38\\python.exe ' + currentdir +'\\AfterBuild_Copy_bins_to_AWS_Server.py')
    os.system('c:\\Users\\carmb\\AppData\\Local\\Programs\\Python\\Python38\\python.exe ' + currentdir +'\\AfterBuild_Copy_bins_to_AWS_Server.py')
    print("Binaries copied and uploaded.")

def after_upload(source, target, env):
    print("after_upload function")
    import os,sys,inspect,subprocess
    currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
    print("currentdir: "+currentdir)
    # do some actions.
    # os.system('python '+currentdir+'\AfterBuild_Copy_bins_to_AWS_Server.py')
    # os.system('python '+currentdir+'\AfterBuild_Copy_bins_to_AWS_Server.py')
    # env.Execute('$PYTHONEXE '  +currentdir+'\AfterBuild_Copy_bins_to_AWS_Server.py')
        # build the spiffs.bin file
    subprocess.run('c:\\Users\\carmb\\AppData\\Local\\Programs\\Python\\Python38\\python.exe ' + currentdir +'\\Prep_create_upload_data_folder.py')
    print("Running mkspiffs")
    create_SPIFFS_Bin()
    print("Ready for uploads/copying - just ran mkspiffs")
    
    print("Okay, that's all for now!")

  
if __name__ != '__main__':
    
    Import("env", "projenv")
    print(__name__ + "running as not __main___")

    # keep pip at the latest
    # env.Execute('$PYTHONEXE  -m pip install --upgrade pip')
    

    # access to global build environment
    print("env:':",env,"'")

    # access to project build environment (is used source files in "src" folder)
    print("projenv:':",projenv,"'")
    
    print("Current build targets", map(str, BUILD_TARGETS))

    # env.AddPreAction("buildprog", before_upload)
    # env.AddPostAction("buildprog", after_upload)
    env.AddPostAction("buildprog", copy_Bins)


if __name__ == '__main__':
#
    print("running as __main___")
    create_build ()
    import _1_Prep_create_upload_data_folder
    # Prep_create_upload_data_folder.extras_create_data_dir()
    import _2_AfterBuild_Copy_bins_to_AWS_Server
    # AfterBuild_Copy_bins_to_AWS_Server.CopyBins()
# Custom actions when building program/firmware
# #

# env.AddPreAction("buildprog", callback...)
# env.AddPostAction("buildprog", callback...)

# #
# # Custom actions for specific files/objects
# #

# env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", [callback1, callback2,...])
# env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", callback...)

# custom action before building SPIFFS image. For example, compress HTML, etc.
# env.AddPreAction("$BUILD_DIR/spiffs.bin", before_upload)

# # custom action for project's main.cpp
# env.AddPostAction("$BUILD_DIR/src/main.cpp.o", callback...)

# # Custom HEX from ELF
# env.AddPostAction(
#     "$BUILD_DIR/${PROGNAME}.elf",
#     env.VerboseAction(" ".join([
#         "$OBJCOPY", "-O", "ihex", "-R", ".eeprom",
#         "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}.hex"
#     ]), "Building $BUILD_DIR/${PROGNAME}.hex")
# )