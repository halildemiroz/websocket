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
#define MAX_CLIENTS 2

typedef struct{
	int clientSocket;
	char name[50];
} ClientArgs;

ClientArgs* clients[MAX_CLIENTS];

pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;

void forwardMessage(int senderSocket, char* message);
void removeClient(int clientSocket);

void* handleClient(void* arg){
	
	ClientArgs* clientInfo = (ClientArgs*)arg;
	
	char buffer[1024];
	char formattedMsg[1024];
	int readSize;

	while((readSize = recv(clientInfo->clientSocket, buffer, 1024, 0)) > 0){
		buffer[readSize] = '\0';

		snprintf(formattedMsg, sizeof(formattedMsg), "\033[34m[%s]\033[0m: %s", clientInfo->name, buffer);

		forwardMessage(clientInfo->clientSocket, formattedMsg);
		
		memset(buffer, 0, 1024);
	}

	printf("Client [%s] disconnected\n", clientInfo->name);
	removeClient(clientInfo->clientSocket);

	free(clientInfo);
	return NULL;

	// free(arg);
	// char buffer[1024];
	//
	// while(1){
	// 	ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
	// 	if(bytesRead > 0){
	// 		buffer[bytesRead] = '\0';
	// 		printf("Client %d: %s", clientSocket, buffer);
	// 		forwardMessage(clientSocket, buffer, bytesRead);
	// 	}else if(bytesRead == 0){
	// 		printf("Client %d disconnected", clientSocket);
	// 		removeClient(clientSocket);
	// 		break;
	// 	}else{
	// 		perror("read");
	// 		removeClient(clientSocket);
	// 		break;
	// 	}
	// }
	// return NULL;
}

void forwardMessage(int senderSocket, char* message){
	
	pthread_mutex_lock(&clientsMutex);

	for(int i = 0; i < MAX_CLIENTS; ++i){
		if(clients[i] != NULL && clients[i]->clientSocket != senderSocket){
			ssize_t bytesWritten = send(clients[i]->clientSocket, message, strlen(message), 0);
			if(bytesWritten == -1){
				perror("write");
			}
		}
	}
	pthread_mutex_unlock(&clientsMutex);
}

void addClient(ClientArgs* newClient){
	pthread_mutex_lock(&clientsMutex);

	for(int i = 0; i < MAX_CLIENTS; ++i){
		if(clients[i] == NULL){
			clients[i] = newClient;
			printf("Client added to slot %d\n", i);
			break;
		}
	}
	
	// if(clientSockets[0] == -1){
	// 	clientSockets[0] = clientSocket;
	// 	printf("Client 1 connected: %d\n", clientSocket);
	// }else if(clientSockets[1] == -1){
	// 	clientSockets[1] = clientSocket;
	// 	printf("Client 2 connected: %d\n", clientSocket);
	// }else{
	// 	printf("Server full");
	// 	close(clientSocket);
	// }
	pthread_mutex_unlock(&clientsMutex);
}

void removeClient(int clientSocket){
	pthread_mutex_lock(&clientsMutex);
	
	for(int i = 0; i < MAX_CLIENTS; ++i){
		if(clients[i] != NULL && clients[i]->clientSocket == clientSocket){
			close(clients[i]->clientSocket);
			clients[i] = NULL;
			break;
		}
	}

	// if(clientSockets[0] == clientSocket)
	// 	clientSockets[0] = -1;
	// else if(clientSockets[1] == clientSocket)
	// 	clientSockets[1] = -1;
	//
	// close(clientSocket);
	pthread_mutex_unlock(&clientsMutex);

}

int main(int argc, char** argv){
	
	socklen_t addrLen, clientAddrLen;
	struct sockaddr_in addr, clientAddr;

	addrLen = sizeof(addr);
	clientAddrLen = sizeof(clientAddr);

	for(int i = 0; i < MAX_CLIENTS; ++i) clients[i] = NULL;
	int opt = 1;

	// Socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		perror("socket");
		return -1;
	}

	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

	while(1){
		
		struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

		int cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if(cfd == -1){
			perror("accept");
			continue;
		}

		ClientArgs* newClient = malloc(sizeof(ClientArgs));
		newClient->clientSocket = cfd;

		int nameRead = recv(cfd, newClient->name, 50,0);
		if(nameRead<=0){
			close(cfd);
			free(newClient);
			continue;
		}

		newClient->name[strcspn(newClient->name, "\n")] = 0;
		
		printf("New Connection: %s\n", newClient->name);

		addClient(newClient);

		pthread_t thread;
		if(pthread_create(&thread, NULL, handleClient, (void*)newClient) != 0) {
				perror("pthread_create");
				removeClient(cfd);
				free(newClient);
		}else{
			pthread_detach(thread);
		}
	}

	// 	if(clientCount == 0)
	// 		pthread_create(&thread1, NULL, handleClient, clientSocketPtr);
	// 	else
	// 		pthread_create(&thread2, NULL, handleClient, clientSocketPtr);
	//
	// 	clientCount++;
	//
	// 	if(clientCount == 1)
	// 		printf("Waiting for second client..\n");
	// 	else if(clientCount == 2)
	// 		printf("Both clients connected\n");
	//
	// }
	//
	// pthread_join(thread1, NULL);
	// pthread_join(thread2, NULL);
	
	// Clean Up
	close(sfd);

	return 0;
}
