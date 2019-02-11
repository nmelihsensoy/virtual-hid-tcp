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
	
	char mode = '0';
	int element = 0;
	// Arguments Handling
	for (int i = 1; i < argc; i++){
		if((strcmp(argv[i], "-k") == 0) || (strcmp(argv[i], "--keyboard") == 0)){
			mode = '1';
			element = i;
		}else if((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--mouse") == 0)){
			mode = '2';
			element = i;
		}else if((strcmp(argv[i], "-mb") == 0) || (strcmp(argv[i], "--mousebutton") == 0)){
			mode = '3';
			element = i;
		}else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)){
			printf("Virtual HID Socket Client\n"); 
			printf("	Usage: client -k 115 \n"); 
			printf("Available Commands: \n"); 
			printf("	-h / --help : Show help\n"); 
			printf("	-k / --keyboard [code]: Sends keyboard event over socket\n");
			printf("	-m / --mouse [X] [Y]: Sends mouse pointer coordinates over socket\n"); 
			printf("	-mb / --mousebutton [code]: Sends mouse button press over socket\n"); 
		}
	}

	int arg_size = strlen(argv[element+1]);
	char *payload = malloc(arg_size);
	int payload_size = arg_size+1;
	*(payload+0) = mode; // set payloads first element as a id
	for(int i=0; i<arg_size; i++){
		payload[i+1] = *(argv[element+1]+i);
	}

	if(mode=='2'){
		int arg2_size = strlen(argv[element+2]);
		payload_size = arg_size+arg2_size+2;
		payload = realloc(payload, payload_size);

		*(payload+arg_size+1) = ':';
		for(int i=arg_size+2; i<=payload_size; i++){
			payload[i] = *(argv[element+2]+i-arg_size-2);
		}		
	}

	//printf("%s", payload);
	send(sock , payload, payload_size, 0);

	usleep(5);

	close(sock);
	free(payload);
	return 0; 
} 
