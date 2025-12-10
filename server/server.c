#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50

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

	// Accept
	int cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if(cfd == -1){
		perror("accept");
		return -1;
	}

	// send message
	while(1){
		char buffer[1024];
		ssize_t bytesRead = read(cfd, buffer, sizeof(buffer) - 1);
		if(bytesRead > 0){
			buffer[bytesRead] = '\0';
			printf("Client: %s", buffer);
			
			char response[1024];
			snprintf(response, sizeof(response), "%s", buffer);

			ssize_t bytesWritten = write(cfd, response, strlen(response));
			if(bytesWritten == -1){
				perror("write");
				return -1;
			}
		} else if(bytesRead == 0){
			printf("Client disconnected\n");
			break;
		} else {
			perror("read");
			return -1;
		}
	}
	// Clean Up
	close(sfd);
	close(cfd);

	return 0;
}
