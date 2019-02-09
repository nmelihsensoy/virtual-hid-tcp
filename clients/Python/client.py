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
time.sleep(0.2)

sendVar = ''
for index, arg in enumerate(sys.argv):
    if arg in ['--keyboard', '-k']:
        sendVar = sys.argv[index+1] + 'k'
    if arg in ['--help', '-h']:
        print("Virtual HID Socket Python Client") 
        print("	Usage: client -k 115 ") 
        print("Available Commands: ")
        print("	-h / --help : Show help")
        print("	-k / --keyboard [code]: Sends keyboard event over socket") 


s.send(sendVar.encode())

time.sleep(0.1)
s.close()