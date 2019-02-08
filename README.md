# Virtual HID over TCP
A HID device emulation which can be controlled remotely over tcp socket connection.You can create virtual joystick, keyboard or mouse and send joystick button, key press and mouse pointer events over network.

Currently server side is written in C and works only on Linux because dependencies.But client side is platform independent so can be written in any platform&language with using tcp socket connection.

## Installation
```c
git clone https://github.com/nmelihsensoy/virtual-hid-tcp.git
cd virtual-hid-tcp/

## Server
gcc client.c -o client //compile
./server //run

## Client
gcc server.c -o server //compile
./client --help //run
```
