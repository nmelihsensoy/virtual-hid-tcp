# Virtual HID over TCP

A HID device emulation which can be controlled remotely over tcp socket connection.You can create joystick, keyboard or mouse device and send events like **joystick axis and buttons, keyboard key presses, mouse pointer movements** over network.

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

```sh
cd clients/C_Cpp/
make
./client --help
```

### Key Press

115 -> Volume Up // 114 -> Volume Down

```c
./client -k 115
```

### Python Client

### Key Press

115 -> Volume Up // 114 -> Volume Down

```python
python client.py -k 114
```