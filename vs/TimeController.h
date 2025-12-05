#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
using namespace std;

class Line;
class Train;

struct Entry
{
	struct Node
	{
		string station;
		int travelTime = 0;   // время движения до следующей станции
		int stopTime = 60;     // время стоянки на текущей станции
	};

	vector<Node> timetable;
};

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

class Schedule
{
private:
	map<string, vector<Entry>> data;

	void loadSchedule(const string& file);
	int parseTime(const string& txt) const;
public:
	Schedule(const string& file = "Schedule.txt") { loadSchedule(file); }

	const map<string, vector<Entry>>& get() const { return data; }
	//const map<string, vector<Entry>>& getDaySchedule() const { return data[(TimeController::getCurrent() / 86400) % 7]; }
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