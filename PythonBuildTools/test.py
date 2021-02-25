import os, shutil, gzip, glob
import sys

local_root_dblBack='C:\\Users\\carmb\\CloudStation'
source = local_root_dblBack+"\\Arduino\\User_Sketches\\PaGoI-MILwright\\data_uncompressed"
target = local_root_dblBack+"\\Arduino\\User_Sketches\\PaGoI-MILwright\\data"
env = {'PROJECTDATA_DIR':'data','PROJECT_DIR':local_root_dblBack }
#add filetypes (extensions only) to be gzipped before uploading. Everything else will be copied directly
filetypes_to_gzip = ['min.js', 'htm', 'min.css']
filetypes_to_exclude = ['js', 'html', 'css', 'txt', 'md','db','bat', 'htm'] # all of file extensions to not copy

files_to_gzip = [] 
files_to_copy = [] 

os.chdir('C:\\Users\\carmb\\CloudStation\\Arduino\\User_Sketches\\PaGoI-MILwright')
os.chdir(source)
for root, dirs, files in os.walk("."):
	# for directory in dirs:
	# 	print (os.path.join(root,directory))
    for filename in files:
        full_filename= os.path.join(root,filename)
        # print(full_filename)
        for ext in filetypes_to_gzip:
            if full_filename.endswith(ext):
                # print (ext, ">>>", filename)
                files_to_gzip.append(full_filename)
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



print('  files to gzip: ' + str(files_to_gzip))

# all_files = glob.glob(os.path.join(source, '*.*'))
# files_to_copy = list(set(all_files) -  set(files_to_exclude))

print('  files to copy: ' + str(files_to_copy))

sys.exit()
#add filetypes (extensions only) to be gzipped before uploading. Everything else will be copied directly
filetypes_to_gzip = ['min.js', 'htm', 'min.css']
filetypes_to_exclude = ['js', 'html', 'css', 'txt', 'md','db']


print('[COPY/GZIP DATA FILES]')

data_dir = env.get('PROJECTDATA_DIR')
data_src_dir = os.path.join(env.get('PROJECT_DIR'), 'data_uncompressed')

if(os.path.exists(data_dir) and not os.path.exists(data_src_dir) ):
    print('  "data" dir exists, "data_src" not found.')
    print('  renaming "' + data_dir + '" to "' + data_src_dir + '"')
    os.rename(data_dir, data_src_dir)

if(os.path.exists(data_dir)):
    print('  Deleting data dir ' + data_dir)
    shutil.rmtree(data_dir)


print('  Re-creating empty data dir ' + data_dir)
os.mkdir(data_dir)
os.chdir(data_src_dir)

files_to_gzip = [] #//TODO: need to walk thru the subdirectories also
for extension in filetypes_to_gzip:
    files_to_gzip.extend(glob.glob(os.path.join(data_src_dir, '*.' + extension)))

print('  files to gzip: ' + str(files_to_gzip))

# //TODO: add files to exclude from copy 


all_files = glob.glob(os.path.join(data_src_dir, '*.*'))
files_to_copy = list(set(all_files) - set(files_to_gzip))

print('  files to copy: ' + str(files_to_copy))
