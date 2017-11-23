#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
namespace server {
	typedef uint16_t ID;
	enum packets {
		p_disconnect,
		p_position,
		p_getPos,
		p_players,
		p_endPlayers
	};
#pragma pack(push, 1)
	struct position {
		float x;
		float y;
		float z;
		float yaw;
	};
#pragma pack(pop)
	std::vector<SOCKET> users;
	std::map<ID, position> players;
	int userNum = 0;
	FD_SET readFds, writeFds, exceptFds;
	FD_SET readSck, writeSck;
	std::mutex mu;
	int disconnectClient(ID id, char * reason = "socket error") {
		printf("Disconnecting client %i; Reason: %s \n", id, reason);
		closesocket(users[id]);
		users[id] = users[userNum - 1];
		users.pop_back();
		userNum--;
		return userNum;
	}
	int recvInt(ID id) {
		char recvData = 0;
		int data;
		while (recvData < 4) {
			char rec = recv(users[id], ((char*)&data) + recvData, 4 - recvData, NULL);
			if (rec == SOCKET_ERROR) disconnectClient(id);
			recvData += rec;
		}
		return ntohl(data);

	}
	int sendInt(ID id, int & data) {
		char sentData = 0;
		data = htonl(data);
		while (sentData < 4) {
			char sent = send(users[id], ((char*)&data) + sentData, 4 - sentData, NULL);
			if (sent == SOCKET_ERROR) disconnectClient(id);
			sentData += sent;
		}
		return data;
	}
	int recvData(char * storage, int len, ID id) {
		uint32_t recvData = 0;
		while (recvData < len) {
			uint32_t rec = recv(users[id], storage + recvData, len - recvData, NULL);
			if (rec == SOCKET_ERROR) disconnectClient(id);
			recvData += rec;
		}
		return recvData;
	}
	int sendData(char * buffer, int len, ID id) {
		uint32_t sentData = 0;
		while (sentData < len) {
			uint32_t sent = send(users[id], buffer + sentData, len - sentData, NULL);
			if (sent == SOCKET_ERROR) disconnectClient(id);
			sentData += sent;
		}
		return sentData;
	}
	int handleIncomingPackets() {
		while (true) {
			FD_ZERO(&readSck);
			FD_ZERO(&writeSck);
			{
				std::lock_guard<std::mutex> guard(mu);
				for (ID i = 0; i < users.size(); i++) {
					FD_SET(users[i], &readSck);
					FD_SET(users[i], &writeSck);
				}
			}
			select(0, &readSck, NULL, NULL, NULL);
			{
				std::lock_guard<std::mutex> guard(mu);
				for (ID i = 0; i < users.size(); i++) {
					if (FD_ISSET(users[i], &readSck)) {
						int packet = recvInt(i);
						if (packet == p_disconnect) {
							disconnectClient(i, "disconnect packet recieved");
						}
						else if (packet == p_position) {
							position pos;
							recvData((char*)&pos.x, sizeof(pos.x), i);
							recvData((char*)&pos.y, sizeof(pos.y), i);
							recvData((char*)&pos.z, sizeof(pos.z), i);
							recvData((char*)&pos.yaw, sizeof(pos.yaw), i);
							players[i] = pos;
							printf("Pos: %f, %f, %f \n", pos.x, pos.y, pos.z);
						}
						else if (packet == p_getPos) {
							printf("get pos called! \n");
							if (FD_ISSET(users[i], &writeFds)) {
								for (auto it = players.begin(); it != players.end(); it++) {
									ID user = it->first;
									int packet = p_players;
									sendInt(user, packet);
									int size = players.size();
									sendInt(user, size);
									for (auto itt = players.begin(); itt != players.end(); itt++) {
										position pos = itt->second;
										sendData(reinterpret_cast<char*>(&pos.x), sizeof(float), user);
										sendData(reinterpret_cast<char*>(&pos.y), sizeof(float), user);
										sendData(reinterpret_cast<char*>(&pos.z), sizeof(float), user);
										sendData(reinterpret_cast<char*>(&pos.yaw), sizeof(float), user);
									}
									packet = p_endPlayers;
									sendInt(user, packet);
								}
							}
						}
					}
				}
			}
		}
		return 0;
	}
	int startup() {
		long success;
		WSAData wsdt;
		success = WSAStartup(MAKEWORD(2, 1), &wsdt);
		SOCKET sck_server;
		SOCKET sck_connection;
		SOCKADDR_IN address, clientAddr;
		sck_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		u_long mode = 1;
		ioctlsocket(sck_server, FIONBIO, &mode);
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_family = AF_INET;
		address.sin_port = htons(8302);
		bind(sck_server, (SOCKADDR*)&address, sizeof(address));
		listen(sck_server, SOMAXCONN);
		int addrSize = sizeof(address);
		std::thread handleMsg(handleIncomingPackets);
		handleMsg.detach();
		printf("Server started!! \n");
		while (true) {
			FD_ZERO(&readFds);
			FD_ZERO(&exceptFds);
			FD_SET(sck_server, &readFds);
			FD_SET(sck_server, &exceptFds);
			select(0, &readFds, NULL, &exceptFds, NULL);
			if (FD_ISSET(sck_server, &readFds)) {
				if ((sck_connection = accept(sck_server, (SOCKADDR*)&address, &addrSize))) {
					std::lock_guard<std::mutex> guard(mu);
					users.push_back(sck_connection);
					printf("Client %i connected! \n", userNum++);
					
				}
			}
			else if (FD_ISSET(sck_server, &exceptFds)) {
				int errorCode = 0;
				int errSize = sizeof(int);
				if (getsockopt(sck_server, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errSize)) {
					printf("Client %i connection error: %i \n", userNum, errorCode);
				}
			}
		}
		return 0;
	}
}