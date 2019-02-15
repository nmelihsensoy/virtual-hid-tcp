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

// Driver code 
int main() { 
	int sockfd; 
	struct sockaddr_in	 servaddr; 

	// Creating socket file descriptor 
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){ 
		error_handling("socket creation failed");
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	
	int n, len; 

    char *hello = "Hello from client"; 
	sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 

	close(sockfd); 
	return 0; 
} 
