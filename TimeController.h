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
	struct Entry 
	{
		string trainID;
		vector<pair<int, string>> timetable;
	};

	map<string, vector<Entry>> data;

	void loadSchedule(const string& file);
	int parseTime(const string& txt) const;
public:
	Schedule(const string& file = "Schedule.txt") { loadSchedule(file); }

	const map<string, vector<Entry>>& get() const { return data; }
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