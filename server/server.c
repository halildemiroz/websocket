#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define LISTEN_BACKLOG 50

int clientSockets[2] = {-1, -1};

pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;

void forwardMessage(int senderSocket, char* message, int messageLength);
void removeClient(int clientSocket);

void* handleClient(void* arg){
	int clientSocket = *(int*)arg;
	free(arg);
	char buffer[1024];

	while(1){
		ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
		if(bytesRead > 0){
			buffer[bytesRead] = '\0';
			printf("Client %d: %s", clientSocket, buffer);
			forwardMessage(clientSocket, buffer, bytesRead);
		}else if(bytesRead == 0){
			printf("Client %d disconnected", clientSocket);
			removeClient(clientSocket);
			break;
		}else{
			perror("read");
			removeClient(clientSocket);
			break;
		}
	}
	return NULL;
}

void forwardMessage(int senderSocket, char* message, int messageLength){
	
	pthread_mutex_lock(&clientsMutex);
	int targetSocket = -1;
	targetSocket = (senderSocket == clientSockets[0] ? clientSockets[1] : clientSockets[0]);

	if(targetSocket != -1){
		ssize_t bytesWritten = write(targetSocket, message, messageLength);
		if(bytesWritten == -1){
			perror("write to target client");
		}else{
			printf("Successfully forwarded %zd bytes\n", bytesWritten);
		}
	}else{
		printf("No other client to forward message to\n");
	}
	pthread_mutex_unlock(&clientsMutex);
}

void addClient(int clientSocket){
	pthread_mutex_lock(&clientsMutex);
	
	if(clientSockets[0] == -1){
		clientSockets[0] = clientSocket;
		printf("Client 1 connected: %d\n", clientSocket);
	}else if(clientSockets[1] == -1){
		clientSockets[1] = clientSocket;
		printf("Client 2 connected: %d\n", clientSocket);
	}else{
		printf("Server full");
		close(clientSocket);
	}
	pthread_mutex_unlock(&clientsMutex);
}

void removeClient(int clientSocket){
	pthread_mutex_lock(&clientsMutex);

	if(clientSockets[0] == clientSocket)
		clientSockets[0] = -1;
	else if(clientSockets[1] == clientSocket)
		clientSockets[1] = -1;
	
	close(clientSocket);
	pthread_mutex_unlock(&clientsMutex);

}

int main(int argc, char** argv){
	
	socklen_t addrLen, clientAddrLen;
	struct sockaddr_in addr, clientAddr;

	addrLen = sizeof(addr);
	clientAddrLen = sizeof(clientAddr);

	// Socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		perror("socket");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(8082);
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
	
	// Bind
	int bindResult = bind(sfd, (struct sockaddr*)&addr, addrLen);
	if(bindResult == -1){
		perror("bind");
		return -1;
	}

	// Listen
	int listenResult = listen(sfd, LISTEN_BACKLOG);
	if(listenResult == -1){
		perror("listen");
		return -1;
	}

	pthread_t thread1, thread2;
	int clientCount = 0;

	printf("Server waiting for clients..\n");

	while(clientCount < 2){
		int cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if(cfd == -1){
			perror("accept");
			continue;
		}

		int* clientSocketPtr = malloc(sizeof(int));
		*clientSocketPtr = cfd;

		addClient(cfd);

		if(clientCount == 0)
			pthread_create(&thread1, NULL, handleClient, clientSocketPtr);
		else
			pthread_create(&thread2, NULL, handleClient, clientSocketPtr);

		clientCount++;

		if(clientCount == 1)
			printf("Waiting for second client..\n");
		else if(clientCount == 2)
			printf("Both clients connected\n");

	}
	//
	// // Accept
	// int cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	// if(cfd == -1){
	// 	perror("accept");
	// 	return -1;
	// }
	//
	// // send message
	// while(1){
	// 	char buffer[1024];
	// 	ssize_t bytesRead = read(cfd, buffer, sizeof(buffer) - 1);
	// 	if(bytesRead > 0){
	// 		buffer[bytesRead] = '\0';
	// 		printf("Client: %s", buffer);
	//
	// 		char response[1024];
	// 		snprintf(response, sizeof(response), "%s", buffer);
	//
	// 		ssize_t bytesWritten = write(cfd, response, strlen(response));
	// 		if(bytesWritten == -1){
	// 			perror("write");
	// 			return -1;
	// 		}
	// 	} else if(bytesRead == 0){
	// 		printf("Client disconnected\n");
	// 		break;
	// 	} else {
	// 		perror("read");
	// 		return -1;
	// 	}
	// }
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	// Clean Up
	close(sfd);

	return 0;
}
