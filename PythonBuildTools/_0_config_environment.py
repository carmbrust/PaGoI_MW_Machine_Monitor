# import os, shutil, gzip, glob
# import sys



# TODO: retreive this from Platformio.ini  - see below 
defaultAppVersion = "21025"
local_root_dblBack='C:\\Users\\carmb\\CloudStation'
project_dir = "\\Arduino\\User_Sketches\\PaGoI-MILwright"
source = local_root_dblBack + project_dir+"\\data_uncompressed"
target = local_root_dblBack + project_dir+"\\data"
sshPrivateKey= "U:\AWS\WindowsWS_PrivateKey.ppk"
env = {'PROJECTDATA_DIR':'data','PROJECT_DIR':local_root_dblBack }
#add filetypes (extensions only) to be gzipped before uploading. Everything else will be copied directly
filetypes_to_gzip = ['min.js', 'htm', 'min.css']
filetypes_to_exclude = ['js', 'html', 'css', 'txt', 'md','db','bat', 'htm'] # all of file extensions to not copy

files_to_gzip = [] 
files_to_copy = [] 

if __name__ == '__main__':
    import _0_Upload_Data_and_Bins