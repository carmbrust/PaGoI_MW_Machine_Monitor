Import ("env")
x = env.get ("PROJECT_DIR")
print(x) 
parts = x.split("\\")
partsLen = len(parts)-1
d = ""
i = 0
while i < partsLen: 
    print(parts[i]) 
    d += parts[i] + "\\"
    i += 1
print(d)    
env.Replace (MKSPIFFSTOOL = d + 'mklittlefs.exe')
