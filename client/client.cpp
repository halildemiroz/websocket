#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define LISTEN_BACKLOG 50

char name[50];

pthread_mutex_t socketMutex = PTHREAD_MUTEX_INITIALIZER;

void* receiveMessages(void* arg){

	int sock = *(int*)arg;
	char buffer[1024];

	while(1){
		ssize_t bytesRead = read(sock, buffer, sizeof(buffer)-1);

		if(bytesRead > 0){
			buffer[bytesRead] = '\0';

			printf("\r\033[K");
			printf("%s\n", buffer);
			printf("\033[35m[%s]:\033[0m ", name);
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
	
	char serverIP[100];
	int serverPort;

	printf("Enter server ip: ");
	scanf("%99s", serverIP);

	printf("Enter port: ");
	scanf("%d", &serverPort);

	getchar();

	socklen_t addrLen;
	struct sockaddr_in addr;

	addrLen = sizeof(addr);

	// Socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		perror("socket");
		return -1;
	}
	
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	if(inet_pton(AF_INET, serverIP, &addr.sin_addr) <= 0){
		printf("Resolving hostname...\n");
		struct hostent *he = gethostbyname(serverIP);

		if(he == NULL){
			printf("Error:");
			return -1;
		}
		memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
	}
	
	printf("Connecting to %s:%d...\n", serverIP, serverPort);

    // Connect
    int connectResult = connect(sfd, (struct sockaddr*)&addr, addrLen);
	if(connectResult == -1){
		perror("connect");
		return -1;
	}
	
	printf("Enter your name: ");
	fgets(name, 50, stdin);

	name[strcspn(name, "\n")] = 0;
	write(sfd,name,strlen(name));

	printf("--- Joined chat as [%s] ---\n", name);

	pthread_t receiveThread;
	pthread_create(&receiveThread, NULL, receiveMessages, &sfd);
	
	char userInput[1024];

	printf("\033[35m[%s]:\033[0m ", name);
	fflush(stdout);
	
	while(1){
		// printf("You: ");
		fgets(userInput, sizeof(userInput), stdin);
		if(strcmp(userInput, "quit\n") == 0) 
			break;
	
		write(sfd, userInput, strlen(userInput));

		printf("\033[35m[%s]:\033[0m ", name);
		fflush(stdout);

	}
	// pthread_join(receiveThread, NULL);

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
