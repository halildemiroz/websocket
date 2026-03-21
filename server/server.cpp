#include <asm-generic/socket.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <picosec.h>

#define PORT 8080
#define BUFFER_SIZE 4096

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


