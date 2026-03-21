#include <iostream>
#include <ranges>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <picosec.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void sendHandShakeResponse(int clientFD, const char* acceptKey){
	char response[256];

	int responseLen = snprintf(response, sizeof(response),
		"HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %s\r\n\r\n",
		acceptKey);

	ssize_t bytesSent = send(clientFD, response, responseLen, 0);
}

bool generateWSAcceptKey(const char* httpRequest, char* acceptKeyOut){
	const char* keyStart = strstr(httpRequest, "Sec-WebSocket-Key: ");
	if(keyStart == NULL)
		return false;
	
	keyStart += 19;
	uint8_t concatBuffer[60];

	memcpy(concatBuffer, keyStart, 24);
	memcpy(concatBuffer + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

	uint8_t sha1Result[20];
	picosec_sha1_hash(concatBuffer, 20, sha1Result);
	picosec_base64_encode(sha1Result, 20, acceptKeyOut);

	return true;

}

int main(){
	int serverFD, clientSocket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	if((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		std::cerr << "[ERROR] Socket could not created" << std::endl;
		return -1;
	}

	if(setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		std::cerr << "[ERROR] setsockopt unsuccessful" << std::endl;
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if(bind(serverFD, (struct sockaddr*)&address, sizeof(address)) < 0){
		std::cerr << "[ERROR] Bind unsuccessful" << std::endl;
		return -1;
	}

	if(listen(serverFD, 10) < 0){
		std::cerr << "[ERROR] Listen unsuccessful" << std::endl;
		return -1;
	}

	std::cout << "Waiting for connection on port " << PORT;
	
	while(1){
		if((clientSocket = accept(serverFD, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
			std::cerr << "[ERROR] Accept unsuccessful" << std::endl;
			continue;
		}

		std::cout << "\n[NEW CONNECTION]\n";
		
		char buffer[BUFFER_SIZE] = {0};
		read(clientSocket, buffer, BUFFER_SIZE);

		std::cout << "HTTP Request\n" << buffer; 
	
		close(clientSocket);
	}
	return 0;
}


