#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT 8080 

void error_handling(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() { 
	int sockfd; 
	char buffer[1024]; 
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){ 
		error_handling("socket creation failed");
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 
	
	// Bind the socket with the server address 
	if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
		error_handling("bind failed");
	} 
	
    while(1){
        int len, n; 
        n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL | SO_REUSEADDR, ( struct sockaddr *) &cliaddr, &len); 
        buffer[n] = '\0';

        printf("Client : %s\n", buffer); 
        usleep(100);
    }
	
	return 0; 
} 
