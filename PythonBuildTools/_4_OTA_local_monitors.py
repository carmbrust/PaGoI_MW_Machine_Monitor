# Filename: Copy_bin_to_Server_ssh.py
# Goal: 
#   1. look for local modules that are reporting on-line:
#       arp -vn -i eth1 | grep ESP > ESP_IP_List.log
#       arp -vn -i eth1 | grep MILw >> ESP_IP_list.log
#   2. Read list into list
        ##types = int, str, int, int, float
        # types = str, str, str
        # filepath = 'ESP_IP_list.log'
        # listOfIP=[]
        # with open(filepath, 'r') as inputFile:
        #    for line in inputFile:
        #         elements = tuple(t(e) for t,e in zip(types, line.split(' ')))
        #         listOfIP.append(elements)
        #         # hostName, ipAddress, macAddress = listOfIP[0], listOfIP[1], listOfIP[2]
        # # pretty print is an alternative to make easier to read 
        # # # pip install pprint
        # # from pprint import pprint
        # # pprint(myList)
        # print(myList)
#   3. Update firmware and SPIFFS to each IP in the list. 
    # for ip in myList:
    #     base=characters_after_last_dot # 192.168.100.base
    #     UpdateESP(base, AppVersion)



# Created: 20210208
# Author: Chris Armbrust
# Revision: 20210208
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
from _2_AfterBuild_Copy_bins_to_AWS_Server import UpdateESP 


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

        ssh_connect.close()
        print ("ssh upload Complete!")

