#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string>
#include<thread>

#pragma comment(lib, "ws2_32.lib")

bool init() {
	WSADATA data2;
	return WSAStartup(MAKEWORD(2, 2), &data2) == 0;
}
void receiveMessages(SOCKET sock) {
	char buffer[4096];
	while (true) {
		int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
		if (bytesReceived <= 0) {
			std::cout << "[Server disconnected]" << std::endl;
			break;
		}
		std::string message(buffer, bytesReceived);
		std::cout << message << std::endl;
	}
}
int main() {
	if (!init()) {
		std::cerr << "Initialization Failed! Quitting..." << std::endl;
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Invalid socket!" << std::endl;
		return 1;
	}

	int port = 65535;
	std::string ip_addr = "127.0.0.1";
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip_addr.c_str(), &(server_addr.sin_addr));

	if (connect(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
		std::cerr << "Address Initialization Failed!" << std::endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	std::string username;
	std::cout << "Enter your username: ";
	std::getline(std::cin, username);

	std::cout << "Connected! Now type any messsage and hit enter.(type quit to exit)" << std::endl;

	std::thread receiver(receiveMessages, sock);
	receiver.detach();

	std::string cInpt;
	while (true) {
		std::getline(std::cin, cInpt);
		if (cInpt == "quit") break;

		std::string message = "[ " + username + " ]" + ":" + cInpt;
		send(sock, message.c_str(), message.length(), 0);

	}
	closesocket(sock);
	WSACleanup();
	return 0;
}