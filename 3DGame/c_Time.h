#pragma once
#ifndef TIMER_H
#define TIMER_H
#include <memory>
#include <time.h>
struct t_clock {
	int minutes;
	int seconds;
	void operator-=(const int & seconds) {
		this->seconds -= abs((long)seconds);
		while (this->seconds < 0) {
			minutes--;
			this->seconds += 60;
		}
	}
	void operator+=(const int & seconds) {
		this->seconds += abs((long)seconds);
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
	bool operator<=(const int & time) {
		if (minutes <= time) return true;
		else if (minutes == time && seconds <= 0) return true;
		return false;
	}
	bool operator!=(const int & time) {
		if (minutes == time) return false;
		else if (minutes == time && seconds == 0) return false;
		return true;
	}
	bool operator==(const int & time) {
		return minutes == time && seconds == 0;
	}
};
class Timer {
private:
	t_clock cl;
	time_t _time;
	time_t timeNow;
public:
	Timer(int minutes) {
		cl.minutes = minutes;
		cl.seconds = 0;
		time(&_time);
	}
	t_clock getTime() {
		time(&timeNow);
		int diffSeconds = difftime(timeNow, _time);
		cl -= diffSeconds;
		return cl;

	}
	time_t getElapsed() {
		time(&timeNow);
		return difftime(timeNow, _time);
	}
	void operator+=(const int & seconds) {
		cl += seconds;
	}
	void operator-=(const int & seconds) {
		cl -= seconds;
	}
	void operator=(const t_clock & setTime) {
		cl = setTime;
		time(&_time);
	}
};
#endif //TIMER_H