/*
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "client.c"
 * Desc: Virtual HID Socket Client for C
 * 
 * Input Event codes can be found here
 * https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
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

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
		printf("Connection Failed \n"); 
		exit(EXIT_FAILURE); 
	}
	
	// Arguments Handling
	for (int i = 1; i < argc; i++){
		if((strcmp(argv[i], "-k") == 0) || (strcmp(argv[i], "--keyboard") == 0)){
			char *payload =  strcat(argv[i+1], "k");
			send(sock , payload, strlen(payload), 0);
			usleep(10);
		}else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)){
			printf("Virtual HID Socket Client\n"); 
			printf("	Usage: client -k 115 \n"); 
			printf("Available Commands: \n"); 
			printf("	-h / --help : Show help\n"); 
			printf("	-k / --keyboard [code]: Sends keyboard event over socket\n"); 
		}
	}

	close(sock);
	return 0; 
} 
