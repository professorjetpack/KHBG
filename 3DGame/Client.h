#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <cstdio>
#include <vector>
#include "c_Time.h"
#define SCK_CLOSED -8300
#define SCK_FD_NOT_SET -3200
#define SCK_BLOCK 0
#define SCK_NON_BLOCK 1
#define P_END_RECV_ARR -35.02f
#define GAME_PLAYER_MOVER 1
#define GAME_PLAYER_FAST_MOVER 2
#define GAME_PLAYER_PLAY 4
typedef unsigned char byte;
struct position {
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	byte movement;
	double lastMove;
	bool allied;
};
struct arrow_packet {
	float x, y, z;
	float velX, velY, velZ;
	double startClock;
	double clock;
	int shooter;
	unsigned long long arrowId;
	bool newShot;
	bool isLive;
};
namespace client {
	typedef int Packet;
	SOCKET sck_connection;
	SOCKADDR_IN serverAddr;
	FD_SET read, write;
	bool closed = false;
	u_long mode;
#define switchMode(MODE) \
		mode = MODE; \
		ioctlsocket(sck_connection, FIONBIO, &mode);
#define IF(condition) if((condition) == SCK_CLOSED) return SCK_CLOSED
#define V_IF(condition) if((condition) == SCK_CLOSED) return
	unsigned long long lastArrow = 0;
	enum packets {
		p_disconnect,
		p_position,
		p_getPos,
		p_players,
		p_endPlayers,
		p_getArrowId,
		p_finishSendArrow,
		p_getNewArrows,
		p_arrows,
		p_endArrows,
		p_died,
		p_getKills,
		p_getLeader,
		p_name,
		p_getTime,
		p_command,
		p_getName,
		p_getTimeNow,
		p_sendExistingArrow,
		p_getPid,
		p_disableArrow
	};
	int disconnectClient(SOCKET sck) {
		if (closed) return SCK_CLOSED;
		if (FD_ISSET(sck, &write)) {
			Packet packet = p_disconnect;
			packet = htonl(packet);
			send(sck, (char*)&packet, sizeof(packet), NULL);			
			printf("Disconnecting! Error code: %d \n", WSAGetLastError());
		}
		else {
			printf("Unsafe disconnect occured! \n");
		}
		closesocket(sck);
		closed = true;
		WSACleanup();
		return SCK_CLOSED;
	}
	void shutdown() {
		if(!closed) disconnectClient(sck_connection);
	}
	int recvInt(SOCKET id, int & _data) {
		if (closed) return SCK_CLOSED;
		char recvData = 0;
		int data = 0;
		while (recvData < 4) {
			char rec = recv(id, ((char*)&data) + recvData, 4 - recvData, NULL);
			if (rec == SOCKET_ERROR && rec != WSAEWOULDBLOCK) { return disconnectClient(id); }
			recvData += rec;
		}
		_data = ntohl(data);
		return 0;

	}
	int sendInt(SOCKET id, int data) {
		if (closed) return SCK_CLOSED;
		char sentData = 0;
		data = htonl(data);
		while (sentData < 4) {
			char sent = send(id, ((char*)&data) + sentData, 4 - sentData, NULL);
			if (sent == SOCKET_ERROR && sent != WSAEWOULDBLOCK) return disconnectClient(id);
			sentData += sent;
		}
		return 0;
	}
	int recvData(char * storage, int len, SOCKET id) {
		if (closed) return SCK_CLOSED;
		uint32_t recvData = 0;
		while (recvData < len) {
			uint32_t rec = recv(id, storage + recvData, len - recvData, NULL);
			if (rec == SOCKET_ERROR && rec != WSAEWOULDBLOCK) return disconnectClient(id);
			recvData += rec;
		}
		return 0;
	}
	int sendData(char * buffer, int len, SOCKET id) {
		if (closed) return SCK_CLOSED;
		uint32_t sentData = 0;
		while (sentData < len) {
			uint32_t sent = send(id, buffer + sentData, len - sentData, NULL);
			if (sent == SOCKET_ERROR && sent != WSAEWOULDBLOCK) return disconnectClient(id);
			sentData += sent;
		}
		return 0;
	}
	int startup(char * ip) {
		WSAData data;
		WSAStartup(MAKEWORD(2, 1), &data);
		sck_connection = socket(AF_INET, SOCK_STREAM, NULL);
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(8302);
		inet_pton(AF_INET, ip, &serverAddr.sin_addr.s_addr);
		if (connect(sck_connection, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) != 0) {
			printf("Connection error %d \n", WSAGetLastError());
		}
		u_long mode = 1;
		ioctlsocket(sck_connection, FIONBIO, &mode);
		FD_ZERO(&write);
		FD_ZERO(&read);
		FD_SET(sck_connection, &read);
		FD_SET(sck_connection, &write);
		printf("Client started \n");
		return 0;
	}
	void sendCommand(std::string command) {
		printf("Command! \n");
		if (FD_ISSET(sck_connection, &write)) {
			Packet packet = p_command;
			sendInt(sck_connection, packet);
			sendInt(sck_connection, command.size());
			sendData((char*)command.c_str(), command.size(), sck_connection);
			printf("sent command! %s \n", command.c_str());
		}
	}
	void sendName(char * name) {
		if (FD_ISSET(sck_connection, &write)) {
			Packet packet = p_name;
			V_IF(sendInt(sck_connection, packet));
			V_IF(sendInt(sck_connection, strlen(name)));
			V_IF(sendData(name, strlen(name), sck_connection));
		}
	}
	void startLoop() {
		if (closed) return;
		FD_ZERO(&write);
		FD_ZERO(&read);
		FD_SET(sck_connection, &read);
		FD_SET(sck_connection, &write);
//		printf("Cleared fdsets \n");
	}
	int sendPos(position & pos) {
		if (closed) return SCK_CLOSED;
		if (FD_ISSET(sck_connection, &write)) {
			Packet packet = p_position;
			int size = sizeof(pos);
			IF(sendInt(sck_connection, packet));
			IF(sendData((char*)&pos.x, sizeof(pos.x), sck_connection));
			IF(sendData((char*)&pos.y, sizeof(pos.y), sck_connection));
			IF(sendData((char*)&pos.z, sizeof(pos.z), sck_connection));
			IF(sendData((char*)&pos.yaw, sizeof(pos.yaw), sck_connection));
			IF(sendData((char*)&pos.pitch, sizeof(pos.pitch), sck_connection));
			IF(sendData((char*)&pos.movement, sizeof(pos.movement), sck_connection));
//			printf("Sent position \n");
		}
		return 0;
	}
	int stopArrow(unsigned long long arrowId) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_disableArrow;
		IF(sendInt(sck_connection, packet));
		IF(sendData((char*)&arrowId, sizeof(unsigned long long), sck_connection));
		return 0;

	}
	int getNewArrowId(unsigned long long & id) {
		if (closed) return SCK_CLOSED;
		if (FD_ISSET(sck_connection, &write)) {
			Packet p = p_getArrowId;
			IF(sendInt(sck_connection, p));
			switchMode(SCK_BLOCK);
			IF(recvData((char*)&id, sizeof(unsigned long long), sck_connection));
			switchMode(SCK_NON_BLOCK);
			return 0;
		}
		else {
			return SCK_FD_NOT_SET;
		}
		return 0;
	}
	int sendExistingArrow(const arrow_packet & pack) {
		if (closed) return SCK_CLOSED;
		if (FD_ISSET(sck_connection, &write)) {
			Packet p = p_sendExistingArrow;
			IF(sendInt(sck_connection, p));
			IF(sendData((char*)&pack.x, sizeof(pack.x), sck_connection));
			IF(sendData((char*)&pack.y, sizeof(pack.y), sck_connection));
			IF(sendData((char*)&pack.z, sizeof(pack.z), sck_connection));
			IF(sendData((char*)&pack.velX, sizeof(pack.velX), sck_connection));
			IF(sendData((char*)&pack.velY, sizeof(pack.velY), sck_connection));
			IF(sendData((char*)&pack.velZ, sizeof(pack.velZ), sck_connection));
			IF(sendData((char*)&pack.clock, sizeof(pack.clock), sck_connection));
			IF(sendData((char*)&pack.newShot, sizeof(pack.newShot), sck_connection));
			IF(sendData((char*)&pack.isLive, sizeof(pack.isLive), sck_connection));
			IF(sendData((char*)&pack.arrowId, sizeof(pack.arrowId), sck_connection));			
			p = p_finishSendArrow;
			sendInt(sck_connection, p);
			return 0;
		}
		else {
			return SCK_FD_NOT_SET;
		}
	}
	
	int getPid(uint16_t & id) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet p = p_getPid;
		IF(sendInt(sck_connection, p));
		uint16_t pid;
		switchMode(SCK_BLOCK);
		IF(recvData((char*)&pid, sizeof(uint16_t), sck_connection));
		switchMode(SCK_NON_BLOCK);
		id = pid;
		return 0;
	}
	int getArrows(std::vector<arrow_packet> & arrows) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet p = p_getNewArrows;
		IF(sendInt(sck_connection, p));
		startLoop();
		select(0, &read, NULL, NULL, NULL);
		if (FD_ISSET(sck_connection, &read)) {
			IF(recvInt(sck_connection, p));
			if (p != p_arrows) {
				printf("Packed %d recieved instead of p_arrows \n", p);
				disconnectClient(sck_connection);
				return -1;
			}
			switchMode(SCK_BLOCK);
			int amount;
			IF(recvInt(sck_connection, amount));
			bool end = false;
			for (int i = 0; i < amount; i++) {
				arrow_packet arr{};
				IF(recvData((char*)&arr.y, sizeof(float), sck_connection));
				if (arr.y >= P_END_RECV_ARR - 0.5f && arr.y <= P_END_RECV_ARR + 0.5f) {
					end = true;  break;
				}
				IF(recvData((char*)&arr.x, sizeof(float), sck_connection));
				IF(recvData((char*)&arr.z, sizeof(float), sck_connection));
				IF(recvData((char*)&arr.velX, sizeof(float), sck_connection));
				IF(recvData((char*)&arr.velY, sizeof(float), sck_connection));
				IF(recvData((char*)&arr.velZ, sizeof(float), sck_connection));
				IF(recvData((char*)&arr.clock, sizeof(double), sck_connection));
				IF(recvData((char*)&arr.shooter, sizeof(unsigned short), sck_connection));
				IF(recvData((char*)&arr.newShot, sizeof(bool), sck_connection));
				IF(recvData((char*)&arr.isLive, sizeof(bool), sck_connection));		
				IF(recvData((char*)&arr.arrowId, sizeof(unsigned long long), sck_connection));
				arrows.push_back(arr);

			}
			if (!end) {
				float end_packet;
				IF(recvData((char*)&end_packet, sizeof(float), sck_connection));
				if (end_packet <= P_END_RECV_ARR - 0.5f || end_packet >= P_END_RECV_ARR + 0.5f) {
					printf("Packet %f recieved when expecting p_endArrows \n", end_packet);
					disconnectClient(sck_connection);
					return -2;
				}
			}
			switchMode(SCK_NON_BLOCK);

		}
		else {
			return SCK_FD_NOT_SET;
		}
		return 0;
	}
	int notifyDeath(arrow_packet & deathArrow) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_died;
		IF(sendInt(sck_connection, packet));
		IF(sendInt(sck_connection, deathArrow.shooter));
		return 0;
	}
	int getKills(int & kills) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_getKills;
		IF(sendInt(sck_connection, packet));
		switchMode(SCK_BLOCK);
		kills;
		IF(recvInt(sck_connection, kills));
		switchMode(SCK_NON_BLOCK);
		return 0;

	}
	int getLeader(int & leaderKills, std::string & leaderName) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_getLeader;
		IF(sendInt(sck_connection, packet));
		switchMode(SCK_BLOCK);
		int size;
		IF(recvInt(sck_connection, size));
		char * name = new char[size + 1];
		IF(recvData(name, size, sck_connection));
		name[size] = '\0';
		leaderName = name;
		IF(recvInt(sck_connection, leaderKills));
		switchMode(SCK_NON_BLOCK);
		delete[] name;
		return 0;


	}
	int getTime(t_clock & tclock) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_getTime;
		IF(sendInt(sck_connection, packet));
		switchMode(SCK_BLOCK);
		int minutes;
		IF(recvInt(sck_connection, minutes));
		if (minutes == -2) {
			tclock.minutes = minutes;
			switchMode(SCK_NON_BLOCK);
			return 0;
		}
		else {
			int seconds;
			IF(recvInt(sck_connection, seconds));
			tclock.minutes = minutes;
			tclock.seconds = seconds;
		}
		switchMode(SCK_NON_BLOCK);
		return 0;
	}
	int getServerTime(double & elapsed) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_getTimeNow;
		IF(sendInt(sck_connection, packet));
		switchMode(SCK_BLOCK);
		double time;
		IF(recvData((char*)&time, sizeof(double), sck_connection));
		switchMode(SCK_NON_BLOCK);
		elapsed = time;
		return 0;
	}
	int getName(char ** name) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet pack = p_getName;
		IF(sendInt(sck_connection, pack));
		switchMode(SCK_BLOCK);
		int size;
		IF(recvInt(sck_connection, size));
		char * n_name = new char[size + 1];
		IF(recvData(n_name, size, sck_connection));
		n_name[size] = '\0';
		switchMode(SCK_NON_BLOCK);
		*name = n_name;
		return 0;
	}
	int getPos(std::vector<position> & pos) {
		if (closed) return SCK_CLOSED;
		if (!FD_ISSET(sck_connection, &write)) return SCK_FD_NOT_SET;
		Packet packet = p_getPos;
		IF(sendInt(sck_connection, packet));
//		printf("Sent getPos packet \n");
		startLoop();
		select(0, &read, NULL, NULL, NULL);
		if (FD_ISSET(sck_connection, &read)) {
			IF(recvInt(sck_connection, packet));
			if (packet != p_players) {
				printf("Packet %d recieved when expecting p_players! \n", packet);
				return -1;
			}
			switchMode(SCK_BLOCK);
//			printf("Recieved data start packet \n");
			int n;
			IF(recvInt(sck_connection, n));
//			printf("Amount of players: %i \n", n);
			if(pos.size() < n) pos.resize(n);
			for (int i = 0; i < n; i++) {
				IF(recvData(reinterpret_cast<char*>(&(pos[i].x)), sizeof(float), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].y)), sizeof(float), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].z)), sizeof(float), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].yaw)), sizeof(float), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].pitch)), sizeof(float), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].movement)), sizeof(byte), sck_connection));
				IF(recvData(reinterpret_cast<char*>(&(pos[i].allied)), sizeof(bool), sck_connection));
			}
			IF(recvInt(sck_connection, packet));
			if (packet != p_endPlayers) {
				printf("Packet %d recieved when expecting p_endPlayers! \n", packet);
				switchMode(SCK_NON_BLOCK);
				return -2;
			}
			switchMode(SCK_NON_BLOCK);
			return 0;
		}
		else { printf("In getPos() -> Read fdset not set \n"); }
		switchMode(SCK_NON_BLOCK);
		return -3;
		
	}
}