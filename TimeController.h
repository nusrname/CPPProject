#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

class Line;
class Train;

class TimeController 
{
private:
	int currentTime;
	int step;
public:
	TimeController(int time, int step) : currentTime(time), step(step) {}
	void advance();
	string getFormattedTime() const;
	int getCurrent() const;
};

/*static*/ class Schedule
{
private:
	static shared_ptr<map<Line, vector<Train>>> schedule;

	static void loadSchedule(string fileName);
public:
	Schedule() { loadSchedule("Schedule.txt"); }
};

class RandomEventGenerator
{
	//private:
	//	double rushHourProbability;
	//	double normalProbability;
	//	//mt19937 rng;
	//public:
	//	bool isDelayEvent(int currentTime);
	//	int getRandomDelay();
};