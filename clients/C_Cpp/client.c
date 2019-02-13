/*
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "client.c"
 * Desc: Virtual HID Socket Client for C
 * 
 * Input Event codes can be found here
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
 * 
 * 0 - Key Report
 * 1 - Key Press - Any device
 * 2 - Mouse Pointer
 * 
 * Ex: 
 *   Left Click Event -> 11272 -> 1:key press, 1: press val, 272: keycode
 *   X Axis -> 20010 -> 2: pointer, 0: x axis, 010: +10
 *   Y Axis -> 21-10 -> 2: pointer, 1: y axis, -10: -10
 * 
*/

#include <stdio.h> //printf
#include <unistd.h> //usleep, close
#include <stdlib.h> //exit
#include <string.h> //strcmp, memset
#include <arpa/inet.h> //inet_pton
#include <sys/socket.h> 

#define PORT 8080 //Socket Port

int main(int argc, char **argv){

	int sock = 0; 
	struct sockaddr_in serv_addr; 
	char* server_address = "127.0.0.1";

	int mode = 0;
	int element = 0;
	int val = 0;
	char payload[6];
	// Arguments Handling
	for (int i = 1; i < argc; i++){
		if((strcmp(argv[i], "-ip") == 0) || (strcmp(argv[i], "--ipaddress") == 0)){
			server_address = argv[i+1];
		}else if((strcmp(argv[i], "-k") == 0) || (strcmp(argv[i], "--keypress") == 0)){
			mode = 3;
			element = i;
		}else if((strcmp(argv[i], "-pX") == 0) || (strcmp(argv[i], "--pointerX") == 0)){
			mode = 2;
			element = i;
			val = 0;
		}else if((strcmp(argv[i], "-pY") == 0) || (strcmp(argv[i], "--pointerY") == 0)){
			mode = 2;
			element = i;
			val = 1;
		}else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)){
			printf("Virtual HID Socket Client\n"); 
			printf("	Usage: client -k 115 \n"); 
			printf("Available Commands: \n"); 
			printf("	-ip / --ipaddress : Server ip address(Default: 127.0.0.1)");
			printf("	-h / --help : Show help\n"); 
			printf("	-k / --keypress [value]: Sends key press event over socket\n");
			printf("	-pX / --pointerX: Sends X axis coordinates over socket\n"); 
			printf("	-pY / --pointerY [value]: Sends Y axis coordinates over socket\n"); 
		}
	}

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
		printf("Socket creation error \n"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, server_address, &serv_addr.sin_addr)<=0){ 
		printf("\nInvalid address/ Address not supported \n"); 
		exit(EXIT_FAILURE);  
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
		printf("Connection Failed \n"); 
		exit(EXIT_FAILURE); 
	}
	
	sprintf(payload, "%i%i%03d", mode, val, atoi(argv[element+1]));
	send(sock , payload, 6, 0);

	usleep(5);

	close(sock);
	return 0; 
} 
