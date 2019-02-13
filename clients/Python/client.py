'''
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "client.py"
 * Desc: Virtual HID Socket Client for Python
 * 
 * Input Event codes can be found here
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
 * 
'''
#!/usr/bin/env python3
import socket
import time
import sys

HOST = '127.0.0.1'
PORT = 8080

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

payload = ''
mode = 0
val = 0
command_index = 0

for index, arg in enumerate(sys.argv):
    if arg in ['--ipaddress', '-ip']:
        HOST = sys.argv[index+1]
    if arg in ['--keyboard', '-k']:
        mode = 3
        command_index = index
    if arg in ['--pointerX', '-pX']:
        mode = 2
        val = 0
        command_index = index
    if arg in ['--pointerY', '-pY']:
        mode = 2
        val = 1
        command_index = index
    if arg in ['--help', '-h']:
        print("Virtual HID Socket Python Client") 
        print("	Usage: client -k 115 ")
        print("Available Commands: ")
        print(" -ip / --ipaddress : Server ip address(Default: 127.0.0.1)")
        print("	-h / --help : Show help") 
        print("	-k / --keyboard [value]: Sends keypress event over socket")
        print("	-pX / --pointerX: Sends X axis coordinates over socket") 
        print("	-pY / --pointerY [value]: Sends Y axis coordinates over socket")


payload = str(mode) + str(val) + sys.argv[command_index+1]

s.send(payload.encode())

time.sleep(0.1)
s.close()