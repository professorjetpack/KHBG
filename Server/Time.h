#pragma once
#ifndef TIMER_H
#define TIMER_H
#include <memory>
#include <time.h>
struct t_clock {
	int minutes;
	int seconds;
	void operator-=(const clock_t & seconds) {
		int minutes = seconds / 60;
		this->minutes -= minutes;
		this->seconds -= (seconds % 60);
	}
	void operator+=(const clock_t & seconds) {
		this->seconds += seconds;
		while (this->seconds > 59) {
			minutes++;
			this->seconds -= 60;
		}
	}
	bool operator<=(const t_clock & other) {
		if (minutes <= other.minutes) return true;
		else if (minutes == other.minutes && seconds <= other.seconds) return true;
		return false;
	}
};
class Timer {
private:
	t_clock cl;
	clock_t _time;
	clock_t timeNow;
public:
	Timer(int minutes){
		cl.minutes = minutes;
		cl.seconds = 0;
		_time = clock();
	}
	clock_t getElapsed() {
		timeNow = clock();
		return ((double)(timeNow - _time)) / CLOCKS_PER_SEC;
	}
	t_clock getTime() {
		clock_t diffSeconds = getElapsed();
		cl -= diffSeconds;
		return cl;
	}
	void operator+=(const int & seconds) {
		cl += seconds;
	}
	void operator-=(const int & seconds) {
		cl -= seconds;
	}
	void operator=(const t_clock & setTime) {
		cl = setTime;
		_time = clock();
	}
	int mins() {
		return cl.minutes;
	}
	void reset(int minutes) {
		cl.minutes = minutes;
		cl.seconds = 0;
		_time = clock();
	}
};
class ServerTime {
private:
	bool timeStarted;
	clock_t timeStart;
public:
	ServerTime() {
		if (timeStarted == NULL) {
			timeStarted = true;
			timeStart = clock();
		}
	}
	double getSecondsNow() {
		return ((double)clock() - (double)timeStart) / CLOCKS_PER_SEC;
	}
};
#endif //TIMER_H