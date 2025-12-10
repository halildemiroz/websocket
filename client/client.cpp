#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50

pthread_mutex_t socketMutex = PTHREAD_MUTEX_INITIALIZER;

void* receiveMessages(void* arg){

	int sock = *(int*)arg;
	char buffer[1024];
	while(1){
		ssize_t bytesRead = read(sock, buffer, sizeof(buffer)-1);

		if(bytesRead > 0){
			buffer[bytesRead] = '\0';
			printf("\nOther Client: %s\nYou: ", buffer);
			fflush(stdout);
		}else if(bytesRead == 0){
			printf("\nServer disconnected\n");
			break;
		}else{
			perror("read");
			break;
		}
	}
	return NULL;
}


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
	

	pthread_t receiveThread;
	pthread_create(&receiveThread, NULL, receiveMessages, &sfd);
	
	char userInput[1024];
	
	while(1){
		printf("You: ");
		fgets(userInput, sizeof(userInput), stdin);
		if(strcmp(userInput, "quit\n") == 0) 
			break;
	
		write(sfd, userInput, strlen(userInput));

	}
	pthread_join(receiveThread, NULL);

	// Send message
	// while(1){
	// 	// Get user message
	// 	printf("You: ");
	// 	fgets(userInput, sizeof(userInput), stdin);
	// 	if(strcmp(userInput, "quit\n") == 0) 
	// 		break;
	// 	// Send message to server
	// 	ssize_t bytesWritten = write(sfd, userInput, strlen(userInput));
	// 	if(bytesWritten == -1){
	// 		perror("write");
	// 		return -1;
	// 	}
	// 	// Receive respone from server
	// 	ssize_t bytesRead = read(sfd, response, sizeof(response));
	//
	// 	if(bytesRead > 0){
	// 		response[bytesRead] = '\0';
	// 		printf("Server: %s\n", response);
	// 	} else if(bytesRead == 0){
	// 		printf("Server disconnected");
	// 		break;
	// 	} else{
	// 		perror("read");
	// 		break;
	// 	}
	// }
	//
	// Clean Up
	close(sfd);

	return 0;
}
