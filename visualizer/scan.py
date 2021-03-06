import socket
import time
import numpy as np
import copy

sock = socket.socket()
sock.connect(('192.168.43.169', 80)) # lidar hostname or local ip address

head = [0xFF,0xFE,0xFD,0xFC,0xFB,0xFA,0xF9,0xF8,0x0F7,0xF6]
buf_size = 1800
meas_length = 6

f = open("./file.xyz", "w") # path for the output point cloud file (~100MB for each full scan)
percent = 0

while(1):
    i = 0
    t = time.time()
    
    while(i < (len(head))):
        c = int.from_bytes(sock.recv(1), "little")
        if(c == head[i]):
            i = i+1
        else:
            i = 0
    buf = sock.recv(buf_size, socket.MSG_WAITALL)

    print("scan advancement: " + str(percent) + "%") # print scan advancement
    
    for i in range(int(buf_size/meas_length)):
        ze = (buf[i*meas_length+1] + (buf[i*meas_length] << 8))/100
        ra = buf[i*meas_length+3] + (buf[i*meas_length+2] << 8)
        az = (buf[i*meas_length+5] + (buf[i*meas_length+4] << 8))/22.22222222222222222
        
        percent = az/1.8
        
        x = ra * np.sin(np.radians(ze)) * np.cos(np.radians(az))
        y = ra * np.sin(np.radians(ze)) * np.sin(np.radians(az))
        z = ra * np.cos(np.radians(ze))
        
        f.write(str(x) + " " + str(y) + " " + str(z) + "\n")