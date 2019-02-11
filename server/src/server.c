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

#define PORT 8080 //Socket port

struct uinput_setup usetup;

//Key events must be defined in keys[] before using
int keys[] = {BTN_LEFT, BTN_RIGHT, KEY_VOLUMEUP, KEY_VOLUMEDOWN}; 

int server_fd;
int new_socket, valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);


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
        perror("socket failed");
        exit(EXIT_FAILURE);   
    }

    //Attach socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Bind
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);   
    }

    if(listen(server_fd, 3)<0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0){
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while(1){    
        char buffer[1024] = {0};
        valread = recv(new_socket, buffer, 1024, 0);

        if(valread>0){
            int buffer_len = strlen(buffer);
        
            printf("Received: %s Size: %i\n", buffer, buffer_len); //For debugging

            short mode = buffer[0]-'0'; //char to int
            
            buffer[0] = '0'; //Making buffer '0' because atoi giving wrong value
            int incoming_code = atoi(buffer); // char* to int for mode 1 

            /*
            * 1 - Keyboard
            * 2 - Mouse Pointer
            * 3 - Mouse Buttons
            */
            switch (mode){
                case 0:
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                    usleep(5);
                break;
                case 1: 
                    emit(fd, EV_KEY, incoming_code, 1);
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                    emit(fd, EV_KEY, incoming_code, 0);
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                    usleep(5);
                break;
                case 2:         
                    if(incoming_code == 0){
                        emit(fd, EV_REL, REL_X, 1);
                    }else if(incoming_code == 1){
                        emit(fd, EV_REL, REL_Y, 1);
                    }
                    emit(fd, EV_SYN, SYN_REPORT, 0);
                    usleep(5);
                break;
            }
            usleep(2);        
        }else{
            new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        }        
    }

    sleep(1);

    //Detach HID Device
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    //Close socket server
    close(server_fd);
   
    return 0;
}