/*
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "server.c"
 * Desc: Virtual HID over TCP Socket.
 * 
 * This is the server side and written in C.It works only on linux because of the dependencies.
 * But client side can be written any language with using socket connection.
 * 
 * I am using uinput kernel module for HID emulation.There is a more information the following link
 * https://www.kernel.org/doc/html/v4.12/input/uinput.html
 * 
 * Input Event codes can be found here
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
 * 
*/
#include <unistd.h> //usleep, sleep, close, read, write 
#include <stdio.h> //printf, perror
#include <netinet/in.h> //htons, sockaddr_in
#include <string.h> //memset, strcpy, strlen
#include <fcntl.h> //open
#include <stdlib.h> //exit
#include <sys/socket.h>
#include <linux/uinput.h>
#include <netinet/tcp.h>

#define PORT 8080 //Socket port

struct uinput_setup usetup;

//Key events must be defined in keys[] before using
int keys[] = {BTN_LEFT, BTN_RIGHT, KEY_VOLUMEUP, KEY_VOLUMEDOWN}; 

//Socket
int server_fd;
int new_socket, valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char buffer[10];


short mode;
short ev_val;
short neg;
int incoming_code;

void error_handle(char* msg){
    perror(msg);
    exit(EXIT_FAILURE); 
}

/*
 * Sends input events
*/
void emit(int fd, int type, int code, int val){
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

int socket_accept(){
    if((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0){
        error_handle("accept");
        return 0;
    }
    return 1;
}

int main(){
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);  

    //Key events init
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for(int i=0; i<sizeof(keys)/sizeof(int); i++){
        ioctl(fd, UI_SET_KEYBIT, keys[i]);
    }
    
    //Mouse Pointer events init
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "melih-hid");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);
        
    //Create socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
        error_handle("socket failed");
    }

    //Attach socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY | IPPROTO_TCP, &opt, sizeof(opt))){
        error_handle("setsockopt");
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Bind
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        error_handle("bind");  
    }

    if(listen(server_fd, 3)<0){
        error_handle("listen");
    }

    socket_accept();

    while(1){    
        
        valread = recv(new_socket, buffer, 10, 0);

        if(valread>0){
            printf("Received: %s Size: %i\n", buffer, 0); //For debugging

            mode = buffer[0]-'0'; //char to int
            
            buffer[0] = '0'; //Making buffer '0' because atoi giving wrong value
            ev_val = buffer[1]-'0';
            buffer[1] = '0';
            neg = 0;
            if(buffer[2] == '-'){
                neg = 1;
                buffer[2] = '0';
            }

            incoming_code = atoi(buffer); // char* to int for mode 1 
            
            /*
            * 0 - Key Report
            * 1 - Key Press - Any device
            * 2 - Mouse Pointer
            * 
            * Ex: 
            *   Left Click Event -> 11272 -> 1:key press, 1: press val, 272: keycode
            *   X Axis -> 20010 -> 2: pointer, 0: x axis, 010: +10
            *   Y Axis -> 21-10 -> 2: pointer, 1: y axis, -10: -10
            */
            switch (mode){
                case 0:
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                break;
                case 1:
                    emit(fd, EV_KEY, incoming_code, ev_val);
                break;
                case 2:
                    if(neg){
                        incoming_code = -incoming_code;
                    }                      
                    if(ev_val == 0){
                        emit(fd, EV_REL, REL_X, incoming_code);
                    }else if(ev_val == 1){
                        emit(fd, EV_REL, REL_Y, incoming_code);
                    }
                break;
                case 3:
                    emit(fd, EV_KEY, incoming_code, 1);
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                    emit(fd, EV_KEY, incoming_code, 0);
                break;
            }
            emit(fd, EV_SYN, SYN_REPORT, 0);
            usleep(5);        
        }else{
            socket_accept();
        }  
        usleep(10);      
    }

    sleep(1);

    //Detach HID Device
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    //Close socket server
    close(server_fd);
   
    return 0;
}