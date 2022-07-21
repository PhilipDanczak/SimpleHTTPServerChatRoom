#include <iostream>
#include <WS2tcpip.h>
#include <sstream>
#include <vector>
#include <chrono>
#include <ctime>   
#pragma warning(disable : 4996) 
#pragma comment (lib, "ws2_32.lib")
#define BUFFER_SIZE 4096

int main() {

	WSADATA winsock_data;
	WORD version = MAKEWORD(2, 2);
	int winsock_ok = WSAStartup(version, &winsock_data);
	if (winsock_ok != 0) { 	// Do check to see if any errors occurred while creating the WSADATA variable.
		std::cerr << "Unable to Initialize WinSock! Exiting Program." << std::endl;
		return EXIT_FAILURE;
	}

	SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET) { //Do a check to see if we successfully created a socket for the server. 
		std::cerr << "Unable to create a socket for the server! Exiting Program." << std::endl;
		return EXIT_FAILURE;
	}

	sockaddr_in server_details;
	server_details.sin_family = AF_INET;
	server_details.sin_port = htons(54000);
	server_details.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(server_socket, (sockaddr*)&server_details, sizeof(server_details));

	listen(server_socket, SOMAXCONN);

	fd_set master_set = fd_set();
	FD_ZERO(&master_set);

	FD_SET(server_socket, &master_set);

	while (true) {
		fd_set fd_buffer = master_set;
		int socketsCurrentlyConnected = select(0, &fd_buffer, 0, 0 ,0);
		for (size_t i = 0; i < socketsCurrentlyConnected; i++)
		{
			SOCKET transfer_sock = fd_buffer.fd_array[i];
			if (transfer_sock == server_socket) {
				SOCKET client_socket = accept(server_socket, 0, 0);
			
				FD_SET(client_socket, &master_set);
				std::string welcome_message("Welcome to the Chat Room!\r\n");

				send(client_socket, welcome_message.c_str(), welcome_message.size() + 1 , 0);
			}
			else {
				char buffer[BUFFER_SIZE];
				memset(buffer, '0', 0);
				int bytesRecieved = recv(transfer_sock, buffer, BUFFER_SIZE, 0);
				if (bytesRecieved <= 0) {
					closesocket(transfer_sock);
					FD_CLR(transfer_sock, &master_set);
				}
				else {
					for (size_t i = 0; i < master_set.fd_count; i++)
					{
						SOCKET output_socket = master_set.fd_array[i];
						if (output_socket != server_socket && output_socket != transfer_sock) {
							std::ostringstream ss;
							std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							ss  << ctime(&timestamp )<< "\tConnection ID " << transfer_sock << ":" << buffer << "\r\n";
							std::string chat_message = ss.str();
							send(output_socket, chat_message.c_str(), chat_message.size() + 1, 0);
						}
					}
				}
			}
		}
	}
	WSACleanup();
}