import numpy as np

s = ""

with open("/home/nik/kek.xyz", "r") as f:
  for line in f:
    nums = line.split(' ')
    #print(nums)
    
    x1 = float(nums[0])
    y1 = float(nums[1])
    z1 = float(nums[2][:-2])
      
    ra = np.sqrt(np.square(x1) + np.square(y1) + np.square(z1))
    az = np.arctan(y1/x1)
    ze = np.arccos(z1/ra)
    #print(str(ra) + " " + str(az) + " " + str(ze))
    
    """x = ra * np.sin(ze) * np.cos(az)
    y = ra * np.sin(ze) * np.sin(az)
    z = ra * np.cos(ze)"""
    
    x = x1
    y = -y1
    z = z1
    
    #print(np.sin(np.radians(az)))
    s += str(x) + " " + str(y) + " " + str(z) + "\n"

o = open("/home/nik/out.xyz", "w")
o.write(s)
#print(s)"""
