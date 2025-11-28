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
	string trainID;

	struct Node
	{
		enum Type { DEPART, STOP, ARRIVE } type;
		string station;
		int time;
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

static class Schedule
{
private:
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