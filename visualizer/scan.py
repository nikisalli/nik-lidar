import socket
import time
import numpy as np
import copy
import sys
from datetime import datetime
import threading

head = [0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0x0F7, 0xF6]
buf_size = 1800
meas_length = 6
port = 80

# sock.connect(('esp32', 80))  # lidar hostname or local ip address

hostname = socket.gethostname()
local_ip = socket.gethostbyname(hostname).split(".")
local_net = f"{local_ip[0]}.{local_ip[1]}.{local_ip[2]}."

print(f"local network address: {socket.gethostbyname(hostname)}")

print("searching for lidar ...")

found = False
max_threads = 50
main_socket = 0


def start_scan(sck):
    print("trying to connect...")
    opened = False
    percent = 0
    f = 0
    t = time.time()

    while(1):
        i = 0

        while(i < (len(head))):
            c = int.from_bytes(sck.recv(1), "little")
            if(c == head[i]):
                i = i+1
            else:
                i = 0
            if(time.time() - t > 5 and opened is False):
                print("connection failed...")
                return

        buf = sck.recv(buf_size, socket.MSG_WAITALL)

        print("scan advancement: " + str(percent) + "%")  # print scan advancement

        if not opened:
            global found
            found = True
            opened = True
            f = open(f"{datetime.today().strftime('%Y-%m-%d')}.xyz", "w")  # path for the output point cloud file

        for i in range(int(buf_size/meas_length)):
            ze = (buf[i*meas_length+1] + (buf[i*meas_length] << 8))/100
            ra = buf[i*meas_length+3] + (buf[i*meas_length+2] << 8)
            az = (buf[i*meas_length+5] + (buf[i*meas_length+4] << 8))/22.22222222222222222

            percent = az/1.8

            x = ra * np.sin(np.radians(ze)) * np.cos(np.radians(az))
            y = ra * np.sin(np.radians(ze)) * np.sin(np.radians(az))
            z = ra * np.cos(np.radians(ze))

            f.write(str(round(x, 2)) + " " + str(round(y, 2)) + " " + str(round(z, 2)) + "\n")


def check_port(ip, port):
    global found
    print(f"checking {ip}... active threads: {threading.active_count()}")
    try:
        sock = socket.socket()  # TCP
        sock.connect((ip, port))
        start_scan(sock)
    except Exception:
        pass


def validate_ip(s):
    a = s.split('.')
    if len(a) != 4:
        return False
    for x in a:
        if not x.isdigit():
            return False
        i = int(x)
        if i < 0 or i > 255:
            return False
    return True


mode = input("auto mode? y/n: ")
if mode.lower() == 'y':
    for i in range(255):
        if i == str(local_ip[3]):
            print("skipping")
            continue
        threading.Thread(target=check_port, args=[local_net + str(i), port]).start()
        if(not found):
            time.sleep(0.05)

    while threading.active_count() > 1:
        time.sleep(0.5)
    print("auto scan failed.")
else:
    while(True):
        ip = input("insert IP: ")
        if validate_ip(ip):
            try:
                sock = socket.socket()
                sock.connect((ip, port))
                found = True
                print("lidar found at " + ip)
                start_scan(sock)
            except Exception:
                print("cannot connect to lidar on this ip! check if it is connected on the network")
                break
