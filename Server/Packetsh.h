#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <vector>
#include <map>
#include <fstream>
#include <string>
//#include <thread>
//#include <mutex>
#include "Time.h"
#include "Ringbuffer.h"
char comTest[] = "test";
#define IS_DISCONNECT(ID) (send((ID), comTest, sizeof(comTest), NULL) == SOCKET_ERROR)
#define SCK_CLOSED -0xcf1
#define SCK_BLOCK 0
#define SCK_NO_BLOCK 1
#define P_END_SEND_ARR -35.02f
#define GAME_TIME 1
#define GAME_NO_LIMIT 0
#define GAME_TEAMS 2
#define GAME_ADMINS 4
#define MAP_NULL 35000
namespace server {
	typedef unsigned char byte;
	typedef uint16_t ID;
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
#pragma pack(push, 1)
	struct position {
		float x;
		float y;
		float z;
		float yaw;
		float pitch;
		byte movement;
	};
	struct arrow_packet {
		float x, y, z;
		float velX, velY, velZ;
		double clock;
		unsigned long long arrowId;
		bool newShot;
		ID shooter;
		bool isLive;
	};
#pragma pack(pop)
	std::vector<SOCKET> users;
	std::map<ID, position> players;
	std::map<ID, int> kills;
	struct team {
		ID p1;
		ID p2;
		bool operator=(const team & other) {
			return (p1 == other.p1 && p2 == other.p2);
		}
	};
	std::vector<team> teamIds;
	std::vector<ID> adminList;
	std::vector<std::string> banList;
	std::map<ID, std::string> names;
	std::vector<std::string> ips;
	std::map<ID, ID> partners;
	bool teams, admins;
	short password;
	Timer * timer;
	ServerTime time;
	unsigned long long arrowId;
	uint16_t arrowClock = 0;
	Ringbuffer<arrow_packet> newArrows(100);
	int userNum = 0;
	u_long mode;
	FD_SET /*readFds, writeFds,*/ exceptFds;
	FD_SET readSck, writeSck;
#define switchMode(id, MODE) \
	mode = MODE; \
	ioctlsocket(users[id], FIONBIO, &mode);

	void writeBans(const std::vector<std::string> & bans) {
		std::ofstream out;
		out.open("bans.txt", std::ios::out);
		if (!out.is_open()) printf("Bans.txt not open!");
		else {
			for (std::string s : bans) {
				out << s << '\n';
			}
		}
	}
	std::vector<std::string> readBans() {
		std::vector<std::string> buffer;
		std::ifstream in;
		in.open("bans.txt", std::ios::in);
		if (in.is_open()) {
			std::string buf;
			while (std::getline(in, buf)) {
				buffer.push_back(buf);
			}
		}
		return buffer;
	}
//	std::mutex mu;
	int disconnectClient(ID id, char * reason = "socket error") {
//		if (IS_DISCONNECT(id)) return id;
		printf("Disconnecting client %i; Reason: %s \n", id, reason);
		printf("Disconnect error code: %d \n", WSAGetLastError());
		closesocket(users[id]);
		auto it = std::find(users.begin(), users.end(), users[id]);
		if (it != users.end()) users.erase(it);
		else printf("Could not delete user from list \n");
		userNum--;
		kills[id] = 0;
		position p = {3000, 0, 3000, -90.0, 0};
		players[id] = p;
		partners[id] = MAP_NULL;
		int t = -1;
		for (int j = 0; j < teamIds.size(); j++) {
			if (teamIds[j].p1 == id || teamIds[j].p2 == id) {
				t = j;
				break;
			}
		}
		if (t != -1) teamIds.erase(teamIds.begin() + t);
		auto itt = std::find(adminList.begin(), adminList.end(), id);
		if (itt != adminList.end()) adminList.erase(itt);
		return SCK_CLOSED;
	}
/*	void switchMode(ID id, int sckMode) {
		u_long mode = sckMode;
		ioctlsocket(users[id], FIONBIO, &mode);
	}*/
	int recvInt(ID id) {
		char recvData = 0;
		int data;
		while (recvData < 4) {
			char rec = recv(users[id], ((char*)&data) + recvData, 4 - recvData, NULL);
			if (rec == SOCKET_ERROR && rec != WSAEWOULDBLOCK) return disconnectClient(id);
			recvData += rec;
		}
		return ntohl(data);

	}
	int sendInt(ID id, int data) {
		char sentData = 0;
		data = htonl(data);
		while (sentData < 4) {
			char sent = send(users[id], ((char*)&data) + sentData, 4 - sentData, NULL);
			if (sent == SOCKET_ERROR && sent != WSAEWOULDBLOCK) return disconnectClient(id);
			sentData += sent;
		}
		return 0;
	}
	int recvData(char * storage, int len, ID id) {
		uint32_t recvData = 0;
		while (recvData < len) {
			uint32_t rec = recv(users[id], storage + recvData, len - recvData, NULL);
			if (rec == SOCKET_ERROR && rec != WSAEWOULDBLOCK) return disconnectClient(id);
			recvData += rec;
		}
		return recvData;
	}
	int sendData(char * buffer, int len, ID id) {
		uint32_t sentData = 0;
		while (sentData < len) {
			uint32_t sent = send(users[id], buffer + sentData, len - sentData, NULL);
			if (sent == SOCKET_ERROR && sent != WSAEWOULDBLOCK) return disconnectClient(id);
			sentData += sent;
		}
		return sentData;
	}
	int handleIncomingPackets() {
//		while (true) {
/*			FD_ZERO(&readSck);
			FD_ZERO(&writeSck);
			{
				std::lock_guard<std::mutex> guard(mu);
				for (ID i = 0; i < users.size(); i++) {
					FD_SET(users[i], &readSck);
					FD_SET(users[i], &writeSck);
				}
			}*/
//			select(0, &readSck, NULL, NULL, NULL);
			{
//				std::lock_guard<std::mutex> guard(mu);
				for (ID i = 0; i < users.size(); i++) {
					if (FD_ISSET(users[i], &readSck)) {
						int packet = recvInt(i);
						if (packet == p_disconnect || packet == SCK_CLOSED) {
							printf("Disconnect called \n");
							disconnectClient(i, "disconnect packet recieved");
							continue;
						}
						else if (packet == p_getTimeNow) {
							if (!FD_ISSET(users[i], &writeSck)) continue;
							double tNow = time.getSecondsNow();
							if(sendData((char*)&tNow, sizeof(double), i) == SCK_CLOSED) continue;
						}
						else if (packet == p_getPid) {
							if (!FD_ISSET(users[i], &writeSck)) continue;
							printf("Getting pid of %d \n", i);
							ID pid = i;
							if (sendData((char*)&pid, sizeof(ID), i) == SCK_CLOSED) continue;
						}
						else if (packet == p_name) {
							switchMode(i, SCK_BLOCK);
							int nameSize = recvInt(i);
							if (nameSize == SCK_CLOSED) continue;
							char * name = new char[nameSize + 1];
							if(recvData(name, nameSize, i) == SCK_CLOSED) continue;
							switchMode(i, SCK_NO_BLOCK);
							name[nameSize] = '\0';
							std::string username(name);
							for (auto it = names.begin(); it != names.end(); it++) {
								if (username == (*it).second) {
									srand(time.getSecondsNow() / 1000.0);
									int num = rand() % 100;
									username += "000" + std::to_string(userNum) + std::to_string(num);
									break;
								}
							}
							names.insert(std::pair<ID, std::string>(i, username));
							printf("Alias of %d: %s \n", i, username.c_str());
							partners.insert(std::pair<ID, ID>(i, MAP_NULL));
							delete[] name;
						}
						else if (packet == p_position) {
//							printf("Incoming position \n");
							switchMode(i, SCK_BLOCK);
							position pos;
							if(recvData((char*)&pos.x, sizeof(pos.x), i) == SCK_CLOSED) continue;
//							printf("Recieved x; ");
							if(recvData((char*)&pos.y, sizeof(pos.y), i) == SCK_CLOSED) continue;
	//						printf("Recieved y; ");
							if(recvData((char*)&pos.z, sizeof(pos.z), i) == SCK_CLOSED) continue;
//							printf("Recieved z; ");
							if(recvData((char*)&pos.yaw, sizeof(pos.yaw), i) == SCK_CLOSED) continue;
//							printf("Recieved yaw;\n");
							if(recvData((char*)&pos.pitch, sizeof(pos.pitch), i) == SCK_CLOSED) continue;
							if (recvData((char*)&pos.movement, sizeof(pos.movement), i) == SCK_CLOSED) continue;
							players[i] = pos;
//							printf("Pos: %f, %f, %f \n", pos.x, pos.y, pos.z);
							switchMode(i, SCK_NO_BLOCK);
						}
						else if (packet == p_getPos) {
//							printf("get pos called! \n");
							if (FD_ISSET(users[i], &writeSck)) {
								int packet = p_players;
								if(sendInt(i, packet) == SCK_CLOSED) continue;
//								printf("Sent data start packet \n");
								int size = players.size() -1;
								if(sendInt(i, size) == SCK_CLOSED) continue; 
//								printf("Sent size %i \n", size);
								if (players.size() - 1 > 0) {
									bool crashed = false;
									for (auto it = players.begin(); it != players.end(); it++) {
										if (i == it->first) continue;
										position pos = it->second;
										bool allied = false;;
//										if (partners.count(i) != 0 && partners.count(it->first) != 0) {
											if (partners[i] == it->first && partners[it->first] == i) allied = true;
//										}
										if (sendData(reinterpret_cast<char*>(&pos.x), sizeof(float), i) == SCK_CLOSED) { crashed = true; break;}
										if (sendData(reinterpret_cast<char*>(&pos.y), sizeof(float), i) == SCK_CLOSED) { crashed = true; break; }
										if (sendData(reinterpret_cast<char*>(&pos.z), sizeof(float), i) == SCK_CLOSED) { crashed = true; break; }
										if (sendData(reinterpret_cast<char*>(&pos.yaw), sizeof(float), i) == SCK_CLOSED) { crashed = true; break; }
										if (sendData(reinterpret_cast<char*>(&pos.pitch), sizeof(float), i) == SCK_CLOSED) { crashed = true; break; }
										if (sendData(reinterpret_cast<char*>(&pos.movement), sizeof(byte), i) == SCK_CLOSED) { crashed = true; break; }
										if (sendData(reinterpret_cast<char*>(&allied), sizeof(bool), i) == SCK_CLOSED) { crashed = true; break; }
										//									printf("Sent data of: %i to %i \n", it->first, i);
									}
									if (crashed) continue;
								}
								packet = p_endPlayers;
								if(sendInt(i, packet) == SCK_CLOSED) continue;
							}
							else {
								printf("Write fdset not set \n");
							}
						}
						else if (packet == p_getTime) {
							if (!FD_ISSET(users[i], &writeSck)) continue;
							int minutes = -2;
							int seconds = 0;
							if (timer == NULL) { if (sendInt(i, minutes) == SCK_CLOSED) continue; }
							else {
								int cl = timer->getElapsed();
								minutes = timer->mins() - (cl / 60);
								seconds = 60 - (cl % 60);
//								printf("Minutes %d : ", minutes);
//								printf("Seconds %d \n", seconds);
								if (minutes < 0) {
									if (minutes <= -1 && seconds <= 40) {
										timer->reset(timer->mins());
										kills.clear();
									}
									minutes = 0;
									seconds = 0;
								}
								if (sendInt(i, minutes) == SCK_CLOSED) continue;
								if (sendInt(i, seconds) == SCK_CLOSED) continue;

							}

						}
						else if (packet == p_disableArrow) {
							switchMode(i, SCK_BLOCK);
							unsigned long long id;
							recvData((char*)&id, sizeof(long long), i);
							switchMode(i, SCK_NO_BLOCK);
							for (auto it = newArrows.begin(); it != newArrows.end(); it++) {
								if ((*it).arrowId == id) {
									(*it).isLive = false;
									break;
								}
							}
						}
						else if (packet == p_getName) {
							long team = -1;
							for (int j = 0; j < teamIds.size(); j++) {
								if (teamIds[j].p1 == i) team = teamIds[j].p2;
								else if(teamIds[j].p2 == i) team = teamIds[j].p1;
							}
							if (team != -1) {
								char msg[500];
								sprintf_s(msg, 500, "Team| %s : %s", names[i].c_str(), names[team].c_str());
								if (sendInt(i, strlen(msg)) == SCK_CLOSED) continue;
								if (sendData(msg, strlen(msg), i) == SCK_CLOSED) continue;

							}
							else {
								char msg[500];
								sprintf_s(msg, 500, "Solo| %s", names[i].c_str());
								if (sendInt(i, strlen(msg)) == SCK_CLOSED) continue;
								if (sendData(msg, strlen(msg), i) == SCK_CLOSED) continue;
							}
						}
						else if (packet == p_getArrowId) {
							printf("send arrow called \n");
							if(sendData((char*)&arrowId, sizeof(arrowId), i) == SCK_CLOSED) continue;
							arrowId++;
						}
						else if (packet == p_sendExistingArrow) {
							switchMode(i, SCK_BLOCK);
							arrow_packet nArrow{};
							memset(&nArrow, 0, sizeof(nArrow));
							if (recvData((char*)&nArrow.x, sizeof(nArrow.x), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.y, sizeof(nArrow.y), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.z, sizeof(nArrow.z), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.velX, sizeof(nArrow.velX), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.velY, sizeof(nArrow.velY), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.velZ, sizeof(nArrow.velZ), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.clock, sizeof(nArrow.clock), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.newShot, sizeof(nArrow.newShot), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.isLive, sizeof(nArrow.isLive), i) == SCK_CLOSED) continue;
							if (recvData((char*)&nArrow.arrowId, sizeof(nArrow.arrowId), i) == SCK_CLOSED) continue;

							if (int num = recvInt(i) != p_finishSendArrow) {
								char msg[256];
								sprintf_s(msg, 256, "Recieved packet %d instead of p_finishSendArrow! \n");
								disconnectClient(i, msg);
								continue;
							}
							switchMode(i, SCK_NO_BLOCK);
							nArrow.shooter = i;
							bool exist = false;
							for (auto it = newArrows.begin(); it != newArrows.end(); it++) {
								if ((*it).arrowId == nArrow.arrowId) {
									(*it) = nArrow;
									exist = true;
									break;
								}
							}
							if (!exist) {
								printf("Arrow does not exist \n");
								newArrows.addElement(nArrow);
							}
						}
						else if (packet == p_getNewArrows) {
//							printf("get arrows called \n"); 
/*							if (++arrowClock > users.size()) {
								newArrows.erase(newArrows.begin(), newArrows.end());
								arrowClock = 0;
							}*/
							unsigned long long lastArrow;
							switchMode(i, SCK_BLOCK);
							if (recvData((char*)&lastArrow, sizeof(lastArrow), i) == SCK_CLOSED) continue;
							switchMode(i, SCK_NO_BLOCK);
							if (FD_ISSET(users[i], &writeSck)) {
								int packet = p_arrows;
								if (sendInt(i, packet) == SCK_CLOSED) continue;
								int amount = newArrows.getSize();
								if (sendInt(i, amount) == SCK_CLOSED) continue;
//								printf("Amount of arrows %d \n", amount);
								bool crashed = false;
								for (auto it = newArrows.begin(); it != newArrows.end(); it++) {
//									if ((*it).arrowId <= lastArrow || (*it).shooter == i) continue;									
									if (sendData((char*)&((*it).y), sizeof((*it).y), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).x), sizeof((*it).x), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).z), sizeof((*it).z), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).velX), sizeof((*it).velX), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).velY), sizeof((*it).velY), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).velZ), sizeof((*it).velZ), i) == SCK_CLOSED) { crashed = true; break;  }
									if (sendData((char*)&((*it).clock), sizeof((*it).clock), i) == SCK_CLOSED) { crashed = true; break;  }									
									if (teamIds.size() > 0) {
										bool sendTeam = false;
										for (auto t = teamIds.begin(); t != teamIds.end(); t++) {
											if (((*t).p1 == i && (*t).p2 == (*it).shooter) || ((*t).p2 == i && (*t).p1 == (*it).shooter)) { sendTeam = true; break; }
										}
										if (sendTeam) {
											ID team = MAP_NULL;
											if (sendData((char*)&team, sizeof(team), i) == SCK_CLOSED) { crashed = true; break; }
										}
									}
									else {
										if (sendData((char*)&((*it).shooter), sizeof((*it).shooter), i) == SCK_CLOSED) { crashed = true; break; }
									}
									if (sendData((char*)&((*it).newShot), sizeof((*it).newShot), i) == SCK_CLOSED) { crashed = true; break; }
									if (sendData((char*)&((*it).isLive), sizeof((*it).isLive), i) == SCK_CLOSED) {crashed = true; break; }
									if (sendData((char*)&((*it).arrowId), sizeof((*it).arrowId), i) == SCK_CLOSED) { crashed = true; break; }
								}
								if (crashed) continue;
								float end_packet = P_END_SEND_ARR;
								if(sendData((char*)&end_packet, sizeof(end_packet), i) == SCK_CLOSED) continue;
							}
						}
						else if (packet == p_died) {						
							switchMode(i, SCK_BLOCK);
							int killer = recvInt(i);
							if (killer == SCK_CLOSED) continue;
							switchMode(i, SCK_NO_BLOCK);
							if (killer == i) continue;
							if (kills[killer] == 0) {
								kills[killer] = 1;
							}
							else {
								kills[killer] += 1;
							}
							printf("%d was killed by %d \n", i, killer);
//							printf("%d was killed by %d \n", i, killer);

						}
						else if (packet == p_getKills) {
							int k = kills[i];
//							printf("%d has %d kills \n", i, k);
							if(sendInt(i, k) == SCK_CLOSED) continue;
						}
						else if (packet == p_getLeader) {
							std::pair<ID, int> lead = std::make_pair(0, 0);
							bool team = false;
							for (auto it = kills.begin(); it != kills.end(); it++) {
								if ((*it).second > lead.second) {
									lead = *it;
								}
							}
							for (int j = 0; j < teamIds.size(); j++) {
								if (kills[teamIds[j].p1] + kills[teamIds[j].p2] > lead.second);
								team = true;
								lead.first = (ID)j;
								lead.second = kills[teamIds[j].p1] + kills[teamIds[j].p2];
							}
							if (team == false) {
								if (sendInt(i, names[lead.first].size()) == SCK_CLOSED) continue;
								char * name = (char*)(names[lead.first].c_str());
								if (sendData(name, names[lead.first].size(), i) == SCK_CLOSED) continue;
								if (sendInt(i, kills[lead.first]) == SCK_CLOSED) continue;
							}
							else {
								char name[256];
								sprintf_s(name, 256, "%s and %s", names[teamIds[lead.first].p1].c_str(), names[teamIds[lead.first].p2].c_str());
//								printf("Leaders are %s and %s \n", names[teamIds[lead.first].p1], names[teamIds[lead.first].p2]);
								if (sendInt(i, strlen(name)) == SCK_CLOSED) continue;
								if (sendData(name, strlen(name), i) == SCK_CLOSED) continue;
								if (sendInt(i, lead.second) == SCK_CLOSED) continue;
							}
						}
						else if (packet == p_command) {
							printf("Command! ");
							switchMode(i, SCK_BLOCK);
							int size = recvInt(i);
							if (size == SCK_CLOSED) continue;
							printf("of size %d ", size);
							char * command = new char[size+1];
							if (recvData(command, size, i) == SCK_CLOSED) { delete[] command; continue; }
							command[size] = '\0';
							printf("that says: %s \n", command);
							switchMode(i, SCK_NO_BLOCK);
							std::string c(command);
							delete[] command;
							printf("Command %s recieved! \n", c.c_str());
							std::string opcode = c.substr(0, c.find(' '));
							if (opcode == "ally" && teams) {
								std::string name = c.substr(c.find(' ') + 1);
								printf("Ally opcode with name %s \n", name.c_str());
//								if (partners[i] == MAP_NULL || partners.count(i) == 0) {
									for (auto it = names.begin(); it != names.end(); it++) {
										if ((*it).second == name) {
											printf("Player exists \n");
											printf("Partnering %d with %d. Partners size: %d \n", i, (*it).first, partners.size());
	//										if (partners[i] == MAP_NULL) partners[i] = (*it).first;
//											/*else*/ partners.insert(std::pair<ID, ID>(i, (*it).first));
											if (i == (*it).first) break;
											partners[i] = (*it).first;
							//				if (partners.count((*it).first) != 0) {
												if (partners[(*it).first] == i) {
													teamIds.push_back({ i, (*it).first });
												}
//											}
	//										partners[i] = (*it).first;
											printf("done. teamIds size: %d  Partners size: %d \n", teamIds.size(), partners.size());
											break;
										}
									}
//								}
//								else printf("In alliance \n");
							}
							else if (opcode == "breakAlliance") {
								partners[i] = MAP_NULL;
								int t = -1;
								for (int j = 0; j < teamIds.size(); j++) {
									if (teamIds[j].p1 == i || teamIds[j].p2 == i) {
										t = j;
										break;
									}
								}
								if (t != -1) teamIds.erase(teamIds.begin() + t);
							}
							else if (opcode == "admin") {
								if (admins) {
									std::string code = c.substr(c.find(' ') + 1);
									short c_password = atoi(code.c_str());
									if (password == c_password) {
										adminList.push_back(i);
										printf("Admin login by: %d \n", i);
									}
								}
							}
							else if (opcode == "kick") {
								if (admins) {
									if (std::find(adminList.begin(), adminList.end(), i) != adminList.end()) {
										std::string name = c.substr(c.find(' ') + 1);
										for (auto it = names.begin(); it != names.end(); it++) {
											if ((*it).second == name) {
												char msg[256];
												sprintf_s(msg, 256, "User %s kicked from server", name.c_str());
												disconnectClient((*it).first, msg);
												break;
											}
										}
									}
								}
							}
							else if (opcode == "ban") {
								if (admins) {
									if (std::find(adminList.begin(), adminList.end(), i) != adminList.end()) {
										std::string name = c.substr(c.find(' ') + 1);
										for (auto it = names.begin(); it != names.end(); it++) {
											if ((*it).second == name) {
												banList.push_back(ips[(*it).first]);
												writeBans(banList);
												char msg[256];
												sprintf_s(msg, 256, "User %s banned from server", name.c_str());
												disconnectClient((*it).first, msg);
												break;
											}
										}
									}
								}
							}

						}
/*						for (auto it = players.begin(); it != players.end(); it++) {
							printf("Pos: %f %f %f \n", it->second.x, it->second.y, it->second.z);
						}
						*/
					}
					else if (FD_ISSET(users[i], &exceptFds)) {
						int errorCode = 0;
						int errorSize = sizeof(int);
						char errorMsg[256] = "fd exception";
						if (getsockopt(users[i], SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorSize)) {
							sprintf_s(errorMsg, 256, "Socket connection error of client %d with error code: %d", i, errorCode);							
						}
						disconnectClient(i, errorMsg);
					}
				}
			}
//		}
		return 0;
	}
	int startup(int gameMode, int time = -1) {
		arrowId = 1;
		long success;
		WSAData wsdt;
		if ((success = WSAStartup(MAKEWORD(2, 1), &wsdt)) != 0) {
			printf("Failes to start server with error code %ld \n", success);
		}
		SOCKET sck_server;
		SOCKET sck_connection;
		SOCKADDR_IN address, clientAddr;
		sck_server = socket(AF_INET, SOCK_STREAM, NULL);
		u_long mode = 1;
		ioctlsocket(sck_server, FIONBIO, &mode);
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_family = AF_INET;
		address.sin_port = htons(8302);
		if (bind(sck_server, (SOCKADDR*)&address, sizeof(address)) != 0) {
			printf("Could not bind on socket with error code: %d \n", WSAGetLastError());
		}
		if (listen(sck_server, SOMAXCONN) != 0) {
			printf("Could not listen on socket with error code: %d \n", WSAGetLastError());
		}
		int addrSize = sizeof(address);
//		std::thread handleMsg(handleIncomingPackets);
//		handleMsg.detach();
		if (gameMode & GAME_TEAMS) {
			teams = true;
		}
		if (gameMode & GAME_ADMINS) {
			admins = true;
			password = gameMode >> 16;
			printf("Password %d \n", password);
			banList = readBans();
		}		
		printf("Server started!! \n");
		while (true) {
			FD_ZERO(&readSck);
			FD_ZERO(&exceptFds);
			FD_ZERO(&writeSck);
			FD_SET(sck_server, &readSck);
			FD_SET(sck_server, &exceptFds);
			for (ID i = 0; i < users.size(); i++) {
				FD_SET(users[i], &readSck);
				FD_SET(users[i], &writeSck);
			}
			select(0, &readSck, NULL, &exceptFds, NULL);
			if (FD_ISSET(sck_server, &readSck)) {
				if ((sck_connection = accept(sck_server, (SOCKADDR*)&address, &addrSize))) {
//					std::lock_guard<std::mutex> guard(mu);
					if (gameMode & GAME_TIME && timer == NULL) {
						timer = new Timer(time);
					}
					getpeername(sck_connection, (SOCKADDR*)&address, &addrSize);
					std::string ip(inet_ntoa(address.sin_addr));
					if (std::find(banList.begin(), banList.end(), ip) == banList.end()) {
						users.push_back(sck_connection);
						ips.push_back(ip);
						printf("Client %i connected (%s)! \n", userNum++, ip.c_str());
					}
					else printf("Client %s is banned! \n", ip.c_str());
					
				}
			}
			else if (FD_ISSET(sck_server, &exceptFds)) {
				int errorCode = 0;
				int errSize = sizeof(int);
				if (getsockopt(sck_server, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errSize)) {
					printf("Client %i connection error: %i \n", userNum, errorCode);
				}
			}
			handleIncomingPackets();
		}
		WSACleanup();
		if(timer != NULL) delete timer;
		return 0;
	}
}