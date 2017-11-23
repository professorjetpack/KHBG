#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <cstdio>
struct position {
	float x;
	float y;
	float z;
	float yaw;
};
namespace client {
	SOCKET sck_connection;
	SOCKADDR_IN serverAddr;
	FD_SET read, write;
	bool closed = false;
	enum packets {
		p_disconnect,
		p_position,
		p_players,
		p_endPlayers
	};
	int disconnectClient(SOCKET sck) {
		if (closed) return sck;
		if (FD_ISSET(sck, &write)) {
			int packet = p_disconnect;
			packet = htonl(packet);
			send(sck, (char*)&packet, sizeof(packet), NULL);			
			printf("Disconnecting! \n");
		}
		else {
			printf("Unsafe disconnect occured! \n");
		}
		closesocket(sck);
		closed = true;
		return sck;
	}
	int recvInt(SOCKET id) {
		if (closed) return -8300;
		char recvData = 0;
		int data;
		while (recvData < 4) {
			char rec = recv(id, ((char*)&data) + recvData, 4 - recvData, NULL);
			if (rec == SOCKET_ERROR) disconnectClient(id);
			recvData += rec;
		}
		return ntohl(data);

	}
	int sendInt(SOCKET id, int & data) {
		if (closed) return -8300;
		char sentData = 0;
		data = htonl(data);
		while (sentData < 4) {
			char sent = send(id, ((char*)&data) + sentData, 4 - sentData, NULL);
			if (sent == SOCKET_ERROR) disconnectClient(id);
			sentData += sent;
		}
		return data;
	}
	int recvData(char * storage, int len, SOCKET id) {
		if (closed) return -8300;
		uint32_t recvData = 0;
		while (recvData < len) {
			uint32_t rec = recv(id, storage + recvData, len - recvData, NULL);
			if (rec == SOCKET_ERROR) disconnectClient(id);
			recvData += rec;
		}
		return recvData;
	}
	int sendData(char * buffer, int len, SOCKET id) {
		if (closed) return -8300;
		uint32_t sentData = 0;
		while (sentData < len) {
			uint32_t sent = send(id, buffer + sentData, len - sentData, NULL);
			if (sent == SOCKET_ERROR) disconnectClient(id);
			sentData += sent;
		}
		return sentData;
	}
	int startup(char * ip) {
		WSAData data;
		WSAStartup(MAKEWORD(2, 1), &data);
		sck_connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(8302);
		inet_pton(AF_INET, ip, &serverAddr.sin_addr.s_addr);
		u_long mode = 1;
		ioctlsocket(sck_connection, FIONBIO, &mode);
		FD_ZERO(&write);
		FD_ZERO(&read);
		FD_SET(sck_connection, &read);
		FD_SET(sck_connection, &write);
		connect(sck_connection, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		select(0, &read, &write, NULL, NULL);
		return 0;
	}
	void startLoop() {
		if (closed) return;
		FD_ZERO(&write);
		FD_ZERO(&read);
		FD_SET(sck_connection, &read);
		FD_SET(sck_connection, &write);
	}
	int sendPos(position pos) {
		if (closed) return -8300;
		if (FD_ISSET(sck_connection, &write)) {
			int packet = p_position;
			int size = sizeof(pos);
			sendInt(sck_connection, packet);
			sendData((char*)&pos.x, sizeof(pos.x), sck_connection);
			sendData((char*)&pos.y, sizeof(pos.y), sck_connection);
			sendData((char*)&pos.z, sizeof(pos.z), sck_connection);
			sendData((char*)&pos.yaw, sizeof(pos.yaw), sck_connection);
		}
		return 0;
	}
	int getPos(std::vector<position> & pos) {
		if (closed) return -4;
		if (FD_ISSET(sck_connection, &read)) {
			int packet = recvInt(sck_connection);
			if (packet != p_players) {
				printf("Packet %d recieved when expecting p_players! \n", packet);
				return -1;
			}
			int n = recvInt(sck_connection);
			pos.resize(n);
			for (int i = 0; i < n; i++) {
				recvData(reinterpret_cast<char*>(&(pos[i].x)), sizeof(float), sck_connection);
				recvData(reinterpret_cast<char*>(&(pos[i].y)), sizeof(float), sck_connection);
				recvData(reinterpret_cast<char*>(&(pos[i].z)), sizeof(float), sck_connection);
				recvData(reinterpret_cast<char*>(&(pos[i].yaw)), sizeof(float), sck_connection);
			}
			if ((packet = recvInt(sck_connection)) != p_endPlayers) {
				printf("Packet %d recieved when expecting p_endPlayers! \n", packet);
				return -2;
			}
			return 0;
		}
		return -3;
		
	}
}