#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class TimeController 
{
private:
	int currentTime;
	int step;
public:
	TimeController(int time, int step) : currentTime(time), step(step) {}
	void advance();
	string getFormattedTime();
	int getCurrent() const;
};

static class Schedule
{

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