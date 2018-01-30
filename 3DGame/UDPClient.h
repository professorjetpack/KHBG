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
#define CLEARMEM(STRUCT) (memset(&(STRUCT), 0, sizeof((STRUCT))))
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
	SOCKET sck;
	SOCKADDR_IN server;
	int serverSize;
	bool closed = false;
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
	int disconnectClient() {
		if (closed) return SCK_CLOSED;
			Packet packet = p_disconnect;
			sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, sizeof(server));
			printf("Disconnecting! Error code: %d \n", WSAGetLastError());
		closesocket(sck);
		closed = true;
		WSACleanup();
		return SCK_CLOSED;
	}
	void shutdown() {
		if (!closed) disconnectClient();
	}
	int startup(char * ip) {
		WSAData data;
		WSAStartup(MAKEWORD(2, 1), &data);
		sck = socket(AF_INET, SOCK_DGRAM, NULL);
		CLEARMEM(server);
		server.sin_family = AF_INET;
		server.sin_port = htons(8302);
		inet_pton(AF_INET, ip, &server.sin_addr.s_addr);
		serverSize = sizeof(server);
		printf("Client started \n");
		return 0;
	}
	void sendCommand(std::string command) {
		printf("Command! \n");
			Packet packet = p_command;
			sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
			int size = command.size();
			sendto(sck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
			sendto(sck, (char*)command.c_str(), size, 0, (SOCKADDR*)&server, serverSize);
			printf("sent command! %s \n", command.c_str());
	}
	int sendName(char * name) {
			Packet packet = p_name;
			sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
			int size = strlen(name);
			sendto(sck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
			sendto(sck, name, size, 0, (SOCKADDR*)&server, serverSize);
			return 0;

	}
	int sendPos(position & pos) {
		if (closed) return SCK_CLOSED;
			Packet packet = p_position;
			int size = sizeof(pos);
			sendto(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
			char buffer[21];
			memcpy_s(buffer, 21, &pos.x, 4);
			memcpy_s(buffer + 4, 17, &pos.y, 4);
			memcpy_s(buffer + 8, 13, &pos.z, 4);
			memcpy_s(buffer + 12, 9, &pos.yaw, 4);
			memcpy_s(buffer + 16, 5, &pos.pitch, 4);
			memcpy_s(buffer + 20, 1, &pos.movement, 1);
			sendto(sck, buffer, 21, 0, (SOCKADDR*)&server, serverSize);
		return 0;
	}
	int stopArrow(unsigned long long arrowId) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_disableArrow;
		sendto(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
		sendto(sck, (char*)&arrowId, sizeof(unsigned long long), 0, (SOCKADDR*)&server, serverSize);
		return 0;

	}
	int getNewArrowId(unsigned long long & id) {
		if (closed) return SCK_CLOSED;
			Packet p = p_getArrowId;
			sendto(sck, (char*)&p, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
			recvfrom(sck, (char*)&id, sizeof(unsigned long long), 0, (SOCKADDR*)&server, &serverSize);
			return 0;
		return 0;
	}
	int sendExistingArrow(const arrow_packet & pack) {
		if (closed) return SCK_CLOSED;
			Packet p = p_sendExistingArrow;
			sendto(sck, (char*)&p, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
			char buffer[42];
			memcpy_s(buffer, 42, &pack.x, 4);
			memcpy_s(buffer + 4, 38, &pack.y, 4);
			memcpy_s(buffer + 8, 34, &pack.z, 4);
			memcpy_s(buffer + 12, 30, &pack.velX, 4);
			memcpy_s(buffer + 16, 26, &pack.velY, 4);
			memcpy_s(buffer + 20, 22, &pack.velZ, 4);
			memcpy_s(buffer + 24, 18, &pack.clock, 8);
			memcpy_s(buffer + 32, 10, &pack.newShot, 1);
			memcpy_s(buffer + 33, 9, &pack.isLive, 1);
			memcpy_s(buffer + 34, 8, &pack.arrowId, 8);
			sendto(sck, buffer, 42, 0, (SOCKADDR*)&server, serverSize);
			p = p_finishSendArrow;
			sendto(sck, (char*)&p, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
			return 0;
	}

	int getPid(uint16_t & id) {
		if (closed) return SCK_CLOSED;
		Packet p = p_getPid;
		sendto(sck, (char*)&p, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
		uint16_t pid;
		recvfrom(sck, (char*)&pid, sizeof(uint16_t), 0, (SOCKADDR*)&server, &serverSize);
		id = pid;
		return 0;
	}
	int getArrows(std::vector<arrow_packet> & arrows) {
		if (closed) return SCK_CLOSED;
		Packet p = p_getNewArrows;
		sendto(sck, (char*)&p, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
			recvfrom(sck, (char*)&p, sizeof(Packet), 0, (SOCKADDR*)&server, &serverSize);
			if (p != p_arrows) {
				printf("Packed %d recieved instead of p_arrows \n", p);
				disconnectClient();
				return -1;
			}
			for (;;) {
				arrow_packet arr{};
				recvfrom(sck, (char*)&arr.y, sizeof(float), 0, (SOCKADDR*)&server, &serverSize);
				if (arr.y >= P_END_RECV_ARR - 0.5f && arr.y <= P_END_RECV_ARR + 0.5f) break;
				char buffer[40];
				recvfrom(sck, buffer, 40, 0, (SOCKADDR*)&server, &serverSize);
				memcpy_s(&arr.x, 4, buffer, 4);
				memcpy_s(&arr.z, 4, buffer + 4, 4);
				memcpy_s(&arr.velX, 4, buffer + 8, 4);
				memcpy_s(&arr.velY, 4, buffer + 12, 4);
				memcpy_s(&arr.velZ, 4, buffer + 16, 4);
				memcpy_s(&arr.clock, 8, buffer + 20, 8);
				memcpy_s(&arr.shooter, 2, buffer + 28, 2);
				memcpy_s(&arr.newShot, 1, buffer + 30, 1);
				memcpy_s(&arr.isLive, 1, buffer + 31, 1);
				memcpy_s(&arr.arrowId, 8, buffer + 32, 8);

				arrows.push_back(arr);

			}
		return 0;
	}
	int notifyDeath(arrow_packet & deathArrow) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_died;
		sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
		sendto(sck, (char*)&(deathArrow.shooter), sizeof(int), 0, (SOCKADDR*)&server, serverSize);
		return 0;
	}
	int getKills(int & kills) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_getKills;
		sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
		int k_kills;
		recvfrom(sck, (char*)&k_kills, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
		kills = k_kills;
		return 0;

	}
	int getLeader(int & leaderKills, std::string & leaderName) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_getLeader;
		sendto(sck, (char*)&packet, sizeof(Packet), 0, (SOCKADDR*)&server, serverSize);
		int size;
		recvfrom(sck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
		char * name = new char[size + 1];
		recvfrom(sck, name, size, 0, (SOCKADDR*)&server, &serverSize);
		name[size] = '\0';
		leaderName = name;
		recvfrom(sck, (char*)&leaderKills, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
		delete[] name;
		return 0;


	}
	int getTime(t_clock & tclock) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_getTime;
		sendto(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
		char time[8];
		recvfrom(sck, time, 8, 0, (SOCKADDR*)&server, &serverSize);
		int minutes;
		memcpy_s((char*)&minutes, sizeof(int), time, sizeof(int));
		if (minutes == -2) {
			tclock.minutes = minutes;
			return 0;
		}
		else {
			int seconds;
			memcpy_s((char*)&seconds, sizeof(int), time + 4, sizeof(int));
			tclock.minutes = minutes;
			tclock.seconds = seconds;
		}
		return 0;
	}
	int getServerTime(double & elapsed) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_getTimeNow;
		sendto(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
		double time;
		recvfrom(sck, (char*)&time, sizeof(double), 0, (SOCKADDR*)&server, &serverSize);
		elapsed = time;
		return 0;
	}
	int getName(char ** name) {
		if (closed) return SCK_CLOSED;
		Packet pack = p_getName;
		sendto(sck, (char*)&pack, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
		int size;
		recvfrom(sck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
		char * n_name = new char[size + 1];
		recvfrom(sck, n_name, size, 0, (SOCKADDR*)&server, &serverSize);
		n_name[size] = '\0';
		*name = n_name;
		return 0;
	}
	int getPos(std::vector<position> & pos) {
		if (closed) return SCK_CLOSED;
		Packet packet = p_getPos;
		sendto(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, serverSize);
			recvfrom(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
			if (packet != p_players) {
				printf("Packet %d recieved when expecting p_players! \n", packet);
				return -1;
			}
			int n;
			recvfrom(sck, (char*)&n, sizeof(n), 0, (SOCKADDR*)&server, &serverSize);
			if (pos.size() < n) pos.resize(n);
			for (int i = 0; i < n; i++) {
				char buffer[22];
				recvfrom(sck, buffer, 22, 0, (SOCKADDR*)&server, &serverSize);
				memcpy_s(&pos[i].x, 4, buffer, 4);
				memcpy_s(&pos[i].y, 4, buffer + 4, 4);
				memcpy_s(&pos[i].z, 4, buffer + 8, 4);
				memcpy_s(&pos[i].yaw, 4, buffer + 12, 4);
				memcpy_s(&pos[i].pitch, 4, buffer + 16, 4);
				memcpy_s(&pos[i].movement, 1, buffer + 20, 1);
				memcpy_s(&pos[i].allied, 1, buffer + 21, 1);
			}
			recvfrom(sck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&server, &serverSize);
			if (packet != p_endPlayers) {
				printf("Packet %d recieved when expecting p_endPlayers! \n", packet);
				return -2;
			}
			return 0;
	}
}