# Virtual HID over TCP
A HID device emulation which can be controlled remotely over tcp socket connection.You can create virtual joystick, keyboard or mouse and send this devices events **Ex: joystick axis and buttons, keyboard key presses, mouse pointer movements** over network.

Server side is written in C and currently works only in Linux.But client side is platform independent so can be written in any platform&language with using tcp socket connection.

Supported event codes are found here : [here](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)

## Installation
```sh
git clone https://github.com/nmelihsensoy/virtual-hid-tcp.git
cd virtual-hid-tcp/

## Server
gcc client.c -o client //compile
./server //run

## Client
gcc server.c -o server //compile
./client --help //run
```

## Usage
### Client Side

#### C/C++ Client

Send key press.
```sh
//115 -> Volume Up
//114 -> Volume Down
./client -k 115
```