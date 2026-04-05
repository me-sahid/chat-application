#include<iostream>
#include<WinSock2.h>
#include<ws2tcpip.h>
#include<tchar.h>
#include<thread>
#include<string>
#include<vector>
#include<mutex>
#pragma comment(lib,"ws2_32.lib")

std::vector<SOCKET>clients;
std::mutex clientMutex;

void broadcast(const std::string& message, SOCKET sendersock) {
	std::lock_guard < std::mutex >lock(clientMutex);
	for (SOCKET client : clients) {
		if (client != sendersock) {
			send(client, message.c_str(), message.length(), 0);
		}
	}
}
void removeClient(SOCKET sock) {
	std::lock_guard < std::mutex >lock(clientMutex);
	clients.erase(std::remove(clients.begin(), clients.end(), sock), clients.end());
	closesocket(sock);
}
void handleClient(SOCKET clientSock) {
	char buffer[4096];

	while (true) {
		int byterecvd = recv(clientSock, buffer, sizeof(buffer), 0);
		if (byterecvd <= 0) {
			std::cout << "[A client disconnected]" << std::endl;
			removeClient(clientSock);
			return;
		}
		std::string message(buffer, byterecvd);
		std::cout << "[Received]" << std::endl;

		broadcast(message, clientSock);
	}
}
bool init() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}
void receiveMesage(SOCKET clientSock) {
	char buffer[4096];
	while (true) {
		int bytereceived = recv(clientSock, buffer, sizeof(buffer), 0);
		if (bytereceived <= 0) {
			std::cout << "[Client Disconnected]!" << std::endl;
			break;
		}
		std::string message(buffer, bytereceived);
		std::cout << "Client: " << message << std::endl;
	}
}
int main() {
	if (!init()) {
		std::cerr << "Initialization failed! Quitting..." << std::endl;
		return 1;
	}
	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == INVALID_SOCKET) {
		std::cerr << "Socket Creation Failed! Quitting..." << std::endl;
		WSACleanup();
		return 1;
	}
	int opt = 1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
	const int port = 65535;
	sockaddr_in sockAddress{};
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(port);

	if (InetPton(AF_INET, _T("0.0.0.0"), &sockAddress.sin_addr) != 1) {
		std::cerr << "Failed setting up address! Quitting..." << std::endl;
		closesocket(listener);
		WSACleanup();
		return 1;
	}
	if (bind(listener, reinterpret_cast<sockaddr*>(&sockAddress), sizeof(sockAddress)) == SOCKET_ERROR) {
		std::cerr << "Bind Failed!" << std::endl;
		closesocket(listener);
		WSACleanup();
		return 1;
	}
	if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listening Failed! Quitting..." << std::endl;
		closesocket(listener);
		WSACleanup();
		return 1;
	}

	std::cout << "Server has started on port: " << port << std::endl;

	while (true) {
		SOCKET clientSock = accept(listener, nullptr, nullptr);
		if (clientSock == INVALID_SOCKET) {
			std::cerr << "[Accept Failed!]";
			continue;
		}
		{
			std::lock_guard < std::mutex >lock(clientMutex);
			clients.push_back(clientSock);
		}
		std::cout << "[New client Connected! Total = " << clients.size() << "]" << std::endl;
		std::thread t(handleClient, clientSock);
		t.detach();

	}
	closesocket(listener);
	WSACleanup();
	return 0;
}