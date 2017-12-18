#include "Packetsh.h"
#include <iostream>
#include <string>
int main() {
	printf("game mode [no_limit] or [timed] \n");
	std::string line;
	std::cin >> line;
	int mode = 0;
	int time;
	if (line == "timed") {
		printf("Time in minutes: \n");
		std::cin >> line;
		time = atoi(line.c_str());
		printf("%d minutes set \n", time);
		mode |= GAME_TIME;
	}
	else {
		mode |= GAME_NO_LIMIT;
		time = -1;
	}
	printf("allow teams [true] or [false] \n");
	line = "";
	std::cin >> line;
	if (line == "true") {
		mode |= GAME_TEAMS;
		printf("teams enabled!");
	}
	server::startup(mode, time);
	return 0;
}