/*
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "device_client.c"
 * 
 * This client sends real device events
*/
#include <stdio.h> //printf
#include <unistd.h> //usleep, close
#include <stdlib.h> //exit
#include <string.h> //strcmp, memset
#include <arpa/inet.h> //inet_pton
#include <fcntl.h>
#include <sys/socket.h> 
#include <linux/input.h>

#define PORT 8080 //Socket Port

struct event_device{
    char *device;
    int fd;
};

int main(int argc, char* argv[]){

    //Socket
    int sock = 0; 
	struct sockaddr_in serv_addr; 

    //HID
    struct input_event ev[64];
    int numevents;
    int result = 0;
    int size = sizeof(struct input_event);
    int rd;
    char name[256];
    char* device[12];
    struct event_device evdevs[12], *evdev;
    int numevdevs = 0;
    fd_set fds;
    int maxfd;

    device[0] = "/dev/input/event12";
    device[1] = "/dev/input/event18";

    for(int i = 0; i < 1; ++i){
        evdev = &evdevs[numevdevs];

        evdev->device = device[i];
        evdev->fd = open(evdev->device, O_RDONLY);
        if (evdev->fd == -1) {
            printf("Failed to open event device: %s.\n", evdev->device);
            continue;
        }
        ++numevdevs;

        memset(name, 0, sizeof(name));
        result = ioctl(evdev->fd, EVIOCGNAME(sizeof(name)), name);
        printf ("Reading From : %s (%s)\n", evdev->device, name);

        printf("Getting exclusive access: ");
        result = ioctl(evdev->fd, EVIOCGRAB, 1);
        printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");
    }

    if(numevdevs == 0){
        exit(1);
    }

    //Socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
		printf("Socket creation error \n"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){ 
		printf("\nInvalid address/ Address not supported \n"); 
		exit(EXIT_FAILURE);  
	} 

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
        printf("Connection Failed \n"); 
        exit(EXIT_FAILURE); 
    }

    while(1){

        FD_ZERO(&fds);
        maxfd = -1;

        for(int i = 0; i < numevdevs; ++i){
            evdev = &evdevs[i];
            FD_SET(evdev->fd, &fds);
            if (maxfd < evdev->fd) maxfd = evdev->fd;
        }

        result = select(maxfd+1, &fds, NULL, NULL, NULL);
        if(result == -1){
            break;        
        }

        for(int i = 0; i < numevdevs; ++i){
            evdev = &evdevs[i];

            if(!FD_ISSET(evdev->fd, &fds)){
                continue;
            }

            if((rd = read(evdev->fd, ev, size * 64)) < size){
                continue;
            }

            numevents = rd / size;
            
            for(int j = 0; j < numevents; ++j){
                printf ("%s: Type[%d] Code[%d] Value[%d]\n", evdev->device, ev[j].type, ev[j].code, ev[j].value);

                char payload[5];
                sprintf(payload, "%d%d", ev[j].type, ev[j].code);

                send(sock , payload, 5, 0);
                usleep(5);
            }
        }
        
    }
    
    for(int i = 0; i < numevdevs; ++i){
        evdev = &evdevs[i];
        result = ioctl(evdev->fd, EVIOCGRAB, 0);
        close(evdev->fd);
    }

    close(sock);

    return 0;
}