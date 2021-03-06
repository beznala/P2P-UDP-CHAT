#pragma comment (lib, "ws2_32.lib")

#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>

std::string Username;
std::vector<std::string> listVictim;
std::vector<int> portVictim;
bool StayConnect = 0;


int resolvehelper(const char* hostname, size_t family, const char* service, sockaddr_storage* pAddr)
{
	int result;
	addrinfo* result_list = NULL;
	addrinfo hints = {};
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	result = getaddrinfo(hostname, service, &hints, &result_list);
	if (result == 0)
	{
		memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
		freeaddrinfo(result_list);
	}

	return result;
}

void receiveMessage(SOCKET T) {
	while (StayConnect) {
		std::vector<char> buffer(4096);
		sockaddr_in client_;
		int client_size = sizeof(client_);
		int receiv_size = 0;
		if ((receiv_size = recvfrom(T, &buffer[0], buffer.size(), 0, (struct sockaddr*)&client_, &client_size)) == SOCKET_ERROR)
			std::cout << "ERROR: Error to receiv message." << std::endl;
		else {
			buffer.resize(receiv_size);
			std::string data(buffer.cbegin(), buffer.cend());
			std::cout << "[" << data.substr(0, data.find("=")) << "]:\t" << data.substr(data.find("=") + 1, data.length()) << std::endl;
		}
	}
}

void sendMessage(SOCKET T) {
	sockaddr_storage addrDest = {};
	std::string send_message;
	while (StayConnect) {
		std::cin >> send_message;

		if (send_message == "101") {
			StayConnect = false;
			break;
		}

		send_message = Username + "=" + send_message;

		for (std::size_t i = 0; i < listVictim.size(); i++) {
			resolvehelper(listVictim[i].c_str(), AF_INET, std::to_string(portVictim[i]).c_str(), &addrDest);
			if (sendto(T, send_message.c_str(), send_message.length(), 0, (struct sockaddr*)&addrDest, sizeof(addrDest)) == SOCKET_ERROR)
				std::cout << "ERROR: Failed to send message." << std::endl;
		}
	}
}

int main()
{
#pragma region Logo
	system("color 5");
	std::cout <<
		"\r\n ________    _______  ________				"
		"\r\n|\\   __  \\  /  ___  \\|\\   __  \\		"
		"\r\n\\ \\  \\|\\  \\/__/|_/  /\\ \\  \\|\\  \\	"
		"\r\n \\ \\   ____\\__|//  / /\\ \\   ____\\	"
		"\r\n  \\ \\  \\___|   /  /_/__\\ \\  \\___|	"
		"\r\n   \\ \\__\\     |\\________\\ \\__\\		"
		"\r\n    \\|__|      \\|_______|\\|__|			"
		"\r\n\r\n"
		<< std::endl;
#pragma endregion

	u_short my_port = 0;
	std::string address = "-1";
	int another = -1;

	std::cout << "Enter your username:\t";	std::cin >> Username;
	std::cout << "Enter your port:\t";	std::cin >> my_port;

	std::cout << "\r\nEnter the data (0 for stop)" << std::endl;
	while (address != "0") {
		std::cout << "Enter the IPv4 address: ";
		std::cin >> address;
		if (address == "0")
			break;
		std::cout << "Enter the port: ";
		std::cin >> another;
		if (another == 0)
			break;
		listVictim.push_back(address);
		portVictim.push_back(another);
	}

	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		std::cout << "ERROR: Error to startup WSA." << std::endl;

	SOCKET T = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKET F = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in client = {};
	client.sin_family = AF_INET;
	client.sin_addr.S_un.S_addr = INADDR_ANY;

	sockaddr_in server = {};
	server.sin_family = AF_INET;
	server.sin_port = htons(my_port);
	server.sin_addr.S_un.S_addr = INADDR_ANY;


	int reuse = 1;
	if (setsockopt(T, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");
	if (setsockopt(F, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");


	if (bind(T, (struct sockaddr*)&client, sizeof(client)) == SOCKET_ERROR)
		std::cout << "ERROR: Failed bind #1" << std::endl;

	if (bind(F, (struct sockaddr*)&server, sizeof(client)) == SOCKET_ERROR)
		std::cout << "ERROR: Failed bind #2" << std::endl;

	std::cout << "\r\nInformation to connect to you:\r\n" << "IPv4:\t" << "Your IPv4 address\r\n"
		<< "Port:\t" << my_port << "\r\nThe connection was successfully established.\r\nFor disconnect write 101.\r\nEnter the message to send...\r\n\r\n" << std::endl;
	StayConnect = true;

	std::thread mh1(receiveMessage, F);
	mh1.detach();

	std::thread mh2(sendMessage, T);
	mh2.join();

	closesocket(T);
	closesocket(F);
	WSACleanup();
}
