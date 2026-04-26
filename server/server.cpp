#include <iostream>
#include <ranges>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <picosec.h>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 4096

void tachyonSendMessage(int clientSocket, const std::string& message){
	std::vector<uint8_t> frame;
	size_t len = message.length();

	frame.push_back(0x81);
	if(len <= 125)
		frame.push_back(static_cast<uint8_t>(len));
	else if(len <= 65535){
		frame.push_back(126);
		frame.push_back((len >> 8) & 0xFF);
		frame.push_back(len & 0xFF);
	}

	frame.insert(frame.end(), message.begin(), message.end());

	send(clientSocket, frame.data(), frame.size(), 0);
}

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

std::string generateWSHandshakeResponse(const std::string& httpREQ){
	const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	std::string searchKey = "Sec-WebSocket-Key: ";
	size_t pos = httpREQ.find(searchKey);

	if(pos == std::string::npos){
		std::cerr << "[ERROR] This is not a WebSocket Request" << std::endl;
		return "";
	}

	std::string clientKey = httpREQ.substr(pos + searchKey.length(), 24);
	std::cout << "[INFO] Pure Key: " << clientKey << std::endl;

	std::string concatStr = clientKey + guid;

	uint8_t sha1HASH[20];
	picosec_sha1_hash(reinterpret_cast<const uint8_t*>(concatStr.c_str()), concatStr.length(), sha1HASH);

	char base64Out[30] = {0};
	picosec_base64_encode(sha1HASH, 20, base64Out);

	std::string response = 
		"HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: " + std::string(base64Out) + "\r\n\r\n";
	
	return response;
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
		
		std::string requestStr(buffer);
		std::string response = generateWSHandshakeResponse(requestStr);
		if(!response.empty()){
			send(clientSocket, response.c_str(), response.length(), 0);
			std::cout << "[INFO] Response: 101 Switching Protocols sent" << std::endl;
		}
	
		while(1){
			memset(buffer, 0, BUFFER_SIZE);
			ssize_t bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
			
			if(bytesRead <= 0){
				std::cout << "[ERROR] Client disconnected" << std::endl;
				break;
			}

			unsigned char opcode = buffer[0] & 0x0F;

			if(opcode == 0x08){
				std::cout << "[INFO] Client wanted to disconnect" << std::endl;
				break;
			}

			bool isMasked = (buffer[1] & 0x80) != 0;
			uint64_t payloadLength = buffer[1] & 0x7F;
			size_t offset = 2;

			if(payloadLength == 126){
				payloadLength = (buffer[2] << 8 | buffer[3]);
				offset = 4;
			}
			else if(payloadLength == 127)
				offset = 10;
			
			if(isMasked){
				unsigned char maskingKey[4];
        maskingKey[0] = buffer[offset];
        maskingKey[1] = buffer[offset + 1];
        maskingKey[2] = buffer[offset + 2];
        maskingKey[3] = buffer[offset + 3];
        offset += 4;

        std::string decodedMsg = "";
                    
        for (uint64_t i = 0; i < payloadLength; i++) {
          char decryptedChar = buffer[offset + i] ^ maskingKey[i % 4];
          decodedMsg += decryptedChar;
        }

        std::cout << "[GELEN MESAJ]: " << decodedMsg << std::endl;
				tachyonSendMessage(clientSocket, "Tachyon Received: " + decodedMsg);
			}
		}
		close(clientSocket);
	}
	return 0;
}

