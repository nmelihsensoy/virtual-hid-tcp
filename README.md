# Virtual HID over TCP

A HID device emulation which can be controlled remotely over network.You can create joystick, keyboard or mouse device and send events like **joystick axis and buttons, keyboard key presses, mouse pointer movements** over network.With new client real device events can be sent.So its perfectly fit for keyboard, mouse sharing. 

Server side is written in C and currently works only in Linux.But client side is platform independent so can be written in any platform&language with using tcp socket connection.

Supported event codes are found here : [here](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)

## Installation

```sh
git clone https://github.com/nmelihsensoy/virtual-hid-tcp.git
cd virtual-hid-tcp/
```

## Server

```sh
cd server/
make
cd bin/
./server
```

## Client

### C/C++ Client

If your server located in diffrent pc then set server ip adress with -ip 11.11.11.11 option.

```sh
cd clients/C_Cpp/
make
./client --help
```

**For Real Mouse Sharing**

```sh
cd clients/C_Cpp/mouse
make
sudo ./mouse_client 127.0.0.1
```

#### Key Press

115 -> Volume Up // 114 -> Volume Down

```sh
./client -k 115
```

#### Mouse Pointer

-pX -pY

```sh
./client -pX 20
```

### Python Client

#### Key Press

115 -> Volume Up // 114 -> Volume Down

```sh
python client.py -k 114
```

#### Mouse Pointer

-pX -pY

```sh
python client.py -pX 20
```