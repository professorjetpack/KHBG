#pragma once
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
#define SCK_CLOSED -0xcf1
#define SCK_BLOCK 0
#define SCK_NO_BLOCK 1
#define P_END_SEND_ARR -35.02f
#define GAME_TIME 1
#define GAME_NO_LIMIT 0
#define GAME_TEAMS 2
#define GAME_ADMINS 4
#define MAP_NULL 35000
namespace udpserver {
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
	SOCKET serverSck;
	std::map<ULONG, ID> users;
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
	unsigned short password;
	Timer * timer;
	ServerTime time;
	unsigned long long arrowId;
	uint16_t arrowClock = 0;
	Ringbuffer<arrow_packet> newArrows(75);
	int userNum = 0;
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
		printf("Disconnecting client %i; Reason: %s \n", id, reason);
		printf("Disconnect error code: %d \n", WSAGetLastError());
		userNum--;
		for (auto it = users.begin(); it != users.end(); it++) {
			if ((*it).second == id) {
				users.erase(it);
				break;
			}
		}
		kills.erase(id);
		players.erase(id);
		partners.erase(id);
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
		names.erase(id);
		return SCK_CLOSED;
	}
	int handleIncomingPackets() {
		{
			SOCKADDR_IN client;
			ZeroMemory(&client, sizeof(client));
			int clientSize = sizeof(client);
			int packet;
			recvfrom(serverSck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&client, &clientSize);
//			if(packet != 14 && packet != 17) printf("Recieved packet %d \n", packet);
			getpeername(serverSck, (SOCKADDR*)&client, &clientSize);
			std::string ip(inet_ntoa(client.sin_addr));
			ULONG expid = client.sin_addr.s_addr;
			if (std::find(banList.begin(), banList.end(), ip) != banList.end()) return 1;
			ID pid = users[expid];
					if (packet == p_disconnect || packet == SCK_CLOSED) {
						disconnectClient(pid, "Disconnect packet recieved");
					}
					else if (packet == p_getTimeNow) {
						double tNow = time.getSecondsNow();
						sendto(serverSck, (char*)&tNow, sizeof(double), 0, (SOCKADDR*)&client, sizeof(client));
					}
					else if (packet == p_getPid) {
						sendto(serverSck, (char*)&pid, sizeof(pid), 0, (SOCKADDR*)&client, clientSize);
					}
					else if (packet == p_name) {
						//first packet that must be sent
						int nameSize;
						recvfrom(serverSck, (char*)&nameSize, sizeof(int), 0, (SOCKADDR*)&client, &clientSize);
						char * name = new char[nameSize + 1];
						recvfrom(serverSck, name, nameSize, 0, (SOCKADDR*)&client, &clientSize);
						name[nameSize] = '\0';
						std::string username(name);
						for (auto it = names.begin(); it != names.end(); it++) {
							if (username == (*it).second) {
								srand(userNum);
								int num = rand() % 100;
								username += "000" + std::to_string(userNum) + std::to_string(num);
								break;
							}
						}
						users.insert(std::make_pair(expid, (ID)userNum));
						pid = userNum++;
						printf("Pid: %d. Usernum: %d \n", pid, userNum);
						names.insert(std::pair<ID, std::string>(pid, username));
						kills.insert(std::pair<ID, int>(pid, 0));
						players.insert(std::pair<ID, position>(pid, { -60, 200, 200, 90, 90, 0 }));
						printf("%s connected (%lu) \n", ip.c_str(), expid);
						printf("Alias of %d: %s \n", users[expid], username.c_str());
						partners.insert(std::pair<ID, ID>(pid, MAP_NULL));
						delete[] name;
					}
					else if (packet == p_position) {
						//							printf("Incoming position \n");
						position pos;
						char buffer[21];
						recvfrom(serverSck, buffer, 21, 0, (SOCKADDR*)&client, &clientSize);
						memcpy_s(&pos.x, 4, buffer, 4);
						memcpy_s(&pos.y, 4, buffer + 4, 4);
						memcpy_s(&pos.z, 4, buffer + 8, 4);
						memcpy_s(&pos.yaw, 4, buffer + 12, 4);
						memcpy_s(&pos.pitch, 4, buffer + 16, 4);
						memcpy_s(&pos.movement, 1, buffer + 20, 1);
						players[pid] = pos;
					}
					else if (packet == p_getPos) {
						//							printf("get pos called! \n");
							int packet = p_players;
							sendto(serverSck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							//								printf("Sent data start packet \n");
							int size = players.size() - 1;
							sendto(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							//								printf("Sent size %i \n", size);
							if (players.size() - 1 > 0) {
								bool crashed = false;
								for (auto it = players.begin(); it != players.end(); it++) {
									if (pid == it->first) continue;
									position pos = it->second;
									bool allied = false;
									if (partners[pid] == it->first && partners[it->first] == pid) allied = true;
									char buffer[22];
									memcpy_s(buffer, 22, &pos.x, 4);
									memcpy_s(buffer + 4, 22 - 4, &pos.y, 4);
									memcpy_s(buffer + 8, 22 - 8, &pos.z, 4);
									memcpy_s(buffer + 12, 22 - 12, &pos.yaw, 4);
									memcpy_s(buffer + 16, 22 - 16, &pos.pitch, 4);
									memcpy_s(buffer + 20, 22 - 20, &pos.movement, 1);
									memcpy_s(buffer + 21, 22 - 21, &allied, 1);
									sendto(serverSck, buffer, 22, 0, (SOCKADDR*)&client, clientSize);
								}
							}
							packet = p_endPlayers;
							sendto(serverSck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
					}
					else if (packet == p_getTime) {
						int minutes = -2;
						int seconds = 0;
						if (timer == NULL) sendto(serverSck, (char*)&minutes, sizeof(int), 0, (SOCKADDR*)&client, clientSize); 
						else {
							int cl = timer->getElapsed();
							minutes = timer->mins() - (cl / 60);
							seconds = 60 - (cl % 60);
							if (minutes < 0) {
								if (minutes <= -1 && seconds <= 40) {
									timer->reset(timer->mins());
									kills.clear();
								}
								minutes = 0;
								seconds = 0;
							}
							char time[8];
							memcpy_s(time, 8, (char*)&minutes, sizeof(int));
							memcpy_s(time + 4, 4, (char*)&seconds, sizeof(int));
							sendto(serverSck, time, 8, 0, (SOCKADDR*)&client, clientSize);

						}

					}
					else if (packet == p_disableArrow) {
						unsigned long long id;
						recvfrom(serverSck, (char*)&id, sizeof(unsigned long long), 0, (SOCKADDR*)&client, &clientSize);
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
							if (teamIds[j].p1 == pid) team = teamIds[j].p2;
							else if (teamIds[j].p2 == pid) team = teamIds[j].p1;
						}
						if (team != -1) {
							char msg[500];
							sprintf_s(msg, 500, "Team| %s : %s", names[pid].c_str(), names[team].c_str());
							int size = strlen(msg);
							sendto(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							sendto(serverSck, msg, size, 0, (SOCKADDR*)&client, clientSize);

						}
						else {
							char msg[500];
							sprintf_s(msg, 500, "Solo| %s", names[pid].c_str());
							int size = strlen(msg);
							sendto(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							sendto(serverSck, msg, size, 0, (SOCKADDR*)&client, clientSize);
						}
					}
					else if (packet == p_getArrowId) {
						printf("get id called \n");
						sendto(serverSck, (char*)&arrowId, sizeof(arrowId), 0, (SOCKADDR*)&client, clientSize);
						arrowId++;
					}
					else if (packet == p_sendExistingArrow) {
						arrow_packet nArrow{};
						memset(&nArrow, 0, sizeof(nArrow));
						char buffer[42];
						recvfrom(serverSck, buffer, 42, 0, (SOCKADDR*)&client, &clientSize);
						memcpy_s(&nArrow.x, 4, buffer, 4);
						memcpy_s(&nArrow.y, 4, buffer + 4, 4);
						memcpy_s(&nArrow.z, 4, buffer + 8, 4);
						memcpy_s(&nArrow.velX, 4, buffer + 12, 4);
						memcpy_s(&nArrow.velY, 4, buffer + 16, 4);
						memcpy_s(&nArrow.velZ, 4, buffer + 20, 4);
						memcpy_s(&nArrow.clock, 8, buffer + 24, 8);
						memcpy_s(&nArrow.newShot, 1, buffer + 32, 1);
						memcpy_s(&nArrow.isLive, 1, buffer + 33, 1);
						memcpy_s(&nArrow.arrowId, 8, buffer + 34, 8);
						int p;
						recvfrom(serverSck, (char*)&p, sizeof(p), 0, (SOCKADDR*)&client, &clientSize);
						if (p != p_finishSendArrow) printf("Expected p_finishSendArrow instead of %d \n", p);
						nArrow.shooter = pid;
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
							int packet = p_arrows;
							sendto(serverSck, (char*)&packet, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
//							int amount = newArrows.getSize();
//							sendto(serverSck, (char*)&amount, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							bool crashed = false;
							for (auto it = newArrows.begin(); it != newArrows.end(); it++) {
								sendto(serverSck, (char*)&((*it).y), sizeof((*it).y), 0, (SOCKADDR*)&client, clientSize);
								char buffer[40];
								ID shooter = (*it).shooter;
								if (teamIds.size() > 0) {
									bool sendTeam = false;
									for (auto t = teamIds.begin(); t != teamIds.end(); t++) {
										if (((*t).p1 == pid && (*t).p2 == (*it).shooter) || ((*t).p2 == pid && (*t).p1 == (*it).shooter)) { shooter = MAP_NULL; break; }
									}
								}
								memcpy_s(buffer, 40, &(*it).x, 4);
								memcpy_s(buffer + 4, 36, &(*it).z, 4);
								memcpy_s(buffer + 8, 32, &(*it).velX, 4);
								memcpy_s(buffer + 12, 28, &(*it).velY, 4);
								memcpy_s(buffer + 16, 24, &(*it).velZ, 4);
								memcpy_s(buffer + 20, 20, &(*it).clock, 8);
								memcpy_s(buffer + 28, 12, &(*it).shooter, 2);
								memcpy_s(buffer + 30, 10, &(*it).newShot, 1);
								memcpy_s(buffer + 31, 9, &(*it).isLive, 1);
								memcpy_s(buffer + 32, 8, &(*it).arrowId, 8);
								sendto(serverSck, buffer, 40, 0, (SOCKADDR*)&client, clientSize);
							}
							float end_packet = P_END_SEND_ARR;
							sendto(serverSck, (char*)&end_packet, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
					}
					else if (packet == p_died) {
						int killer;
						recvfrom(serverSck, (char*)&killer, sizeof(int), 0, (SOCKADDR*)&client, &clientSize);
						if (killer == pid) return 0;
						if (kills[killer] == 0) {
							kills[killer] = 1;
						}
						else {
							kills[killer] += 1;
						}
						printf("%d was killed by %d \n", pid, killer);

					}
					else if (packet == p_getKills) {
						int k = kills[pid];
						sendto(serverSck, (char*)&k, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
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
							if (kills[teamIds[j].p1] + kills[teamIds[j].p2] >= lead.second) {
								team = true;
								lead.first = (ID)j;
								lead.second = kills[teamIds[j].p1] + kills[teamIds[j].p2];
							}
						}
						if (team == false) {
							int size = names[lead.first].size();
							sendto(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							char * name = (char*)(names[lead.first].c_str());
							sendto(serverSck, name, size, 0, (SOCKADDR*)&client, clientSize);
							int ckills = kills[lead.first];
							sendto(serverSck, (char*)&ckills, sizeof(ckills), 0, (SOCKADDR*)&client, clientSize);
						}
						else {
							char name[256];
							sprintf_s(name, 256, "%s and %s", names[teamIds[lead.first].p1].c_str(), names[teamIds[lead.first].p2].c_str());
							int size = strlen(name);
							sendto(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
							sendto(serverSck, name, size, 0, (SOCKADDR*)&client, clientSize);
							int kills = lead.second;
							sendto(serverSck, (char*)&kills, sizeof(int), 0, (SOCKADDR*)&client, clientSize);
						}
					}
					else if (packet == p_command) {
						printf("Command! ");
						int size;
						recvfrom(serverSck, (char*)&size, sizeof(int), 0, (SOCKADDR*)&client, &clientSize);
						printf("of size %d ", size);
						char * command = new char[size + 1];
						recvfrom(serverSck, command, size, 0, (SOCKADDR*)&client, &clientSize);
						command[size] = '\0';
						printf("that says: %s \n", command);
						std::string c(command);
						delete[] command;
						printf("Command %s recieved! ", c.c_str());
						std::string opcode = c.substr(0, c.find(' '));
						printf(" Opcode: %s \n", opcode.c_str());
						if (opcode == "ally" && teams) {
							std::string name = c.substr(c.find(' ') + 1);
							printf("Ally opcode with name %s \n", name.c_str());
							for (auto it = names.begin(); it != names.end(); it++) {
								if ((*it).second == name) {
									printf("Player exists \n");
									printf("Partnering %d with %d. Partners size: %d \n", pid, (*it).first, partners.size());
//									if (pid == (*it).first) break;
									partners[pid] = (*it).first;
									if (partners[(*it).first] == pid) {
										teamIds.push_back({ pid, (*it).first });
									}
									printf("done. teamIds size: %d  Partners size: %d \n", teamIds.size(), partners.size());
									break;
								}
							}
						}
						else if (opcode == "breakAlliance" && teams) {
							partners[pid] = MAP_NULL;
							int t = -1;
							for (int j = 0; j < teamIds.size(); j++) {
								if (teamIds[j].p1 == pid || teamIds[j].p2 == pid) {
									t = j;
									break;
								}
							}
							if (t != -1) teamIds.erase(teamIds.begin() + t);
						}
						else if (opcode == "admin") {
							if (admins) {
								std::string code = c.substr(c.find(' ') + 1);
								unsigned short c_password = atoi(code.c_str());
								if (password == c_password) {
									adminList.push_back(pid);
									printf("Admin login by: %d \n", pid);
								}
							}
						}
						else if (opcode == "kick") {
							if (admins) {
								if (std::find(adminList.begin(), adminList.end(), pid) != adminList.end()) {
									std::string name = c.substr(c.find(' ') + 1);
									for (auto it = names.begin(); it != names.end(); it++) {
										if ((*it).second == name) {
											char msg[256];
											sprintf_s(msg, 256, "User %s kicked from server", name.c_str());
											disconnectClient(pid, msg);
											break;
										}
									}
								}
							}
						}
						else if (opcode == "ban") {
							if (admins) {
								if (std::find(adminList.begin(), adminList.end(), pid) != adminList.end()) {
									std::string name = c.substr(c.find(' ') + 1);
									for (auto it = names.begin(); it != names.end(); it++) {
										if ((*it).second == name) {
											banList.push_back(ips[(*it).first]);
											writeBans(banList);
											char msg[256];
											sprintf_s(msg, 256, "User %s banned from server", name.c_str());
											disconnectClient(pid, msg);
											break;
										}
									}
								}
							}
						}

					}
					else {
						printf("Unknown packet recieved from %d \n", pid);
					}
		}
		return 0;
	}
	int startup(int gameMode, int time = -1) {
		arrowId = 1;
		long success;
		WSAData wsdt;
		if ((success = WSAStartup(MAKEWORD(2, 1), &wsdt)) != 0) {
			printf("Failes to start server with error code %ld \n", success);
		}
		SOCKADDR_IN address;
		serverSck = socket(AF_INET, SOCK_DGRAM, NULL);
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_family = AF_INET;
		address.sin_port = htons(8302);
		if (bind(serverSck, (SOCKADDR*)&address, sizeof(address)) != 0) {
			printf("Could not bind on socket with error code: %d \n", WSAGetLastError());
		}
		int addrSize = sizeof(address);
		if (gameMode & GAME_TEAMS) {
			teams = true;
		}
		if (gameMode & GAME_ADMINS) {
			admins = true;
			password = gameMode >> 16;
			printf("Password %d \n", password);
			banList = readBans();
		}
		if (gameMode & GAME_TIME) {
			timer = new Timer(time);
		}
		printf("Server started!! \n");
		while (true) {
			handleIncomingPackets();
		}
		WSACleanup();
		if (timer != NULL) delete timer;
		return 0;
	}
}