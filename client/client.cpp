#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50

int main(int argc, char** argv){
	
	socklen_t addrLen;
	struct sockaddr_in addr;

	addrLen = sizeof(addr);

	// Socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		perror("socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(8082);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	
    // Connect
    int connectResult = connect(sfd, (struct sockaddr*)&addr, addrLen);
	if(connectResult == -1){
		perror("connect");
		return -1;
	}
	
	char response[1024];
	char userInput[1024];
	// Send message
	while(1){
		// Get user message
		printf("Message: ");
		fgets(userInput, sizeof(userInput), stdin);
		if(strcmp(userInput, "quit\n") == 0) 
			break;
		// Send message to server
		ssize_t bytesWritten = write(sfd, userInput, strlen(userInput));
		if(bytesWritten == -1){
			perror("write");
			return -1;
		}
		// Receive respone from server
		ssize_t bytesRead = read(sfd, response, sizeof(response));
		
		if(bytesRead > 0){
			response[bytesRead] = '\0';
			printf("Server: %s\n", response);
		} else if(bytesRead == 0){
			printf("Server disconnected");
			break;
		} else{
			perror("read");
			break;
		}
	}

	// Clean Up
	close(sfd);

	return 0;
}
