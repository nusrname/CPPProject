#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <thread>
#include <fstream>
#include <sstream>
#include "ConsoleUI.h"
#include "TimeController.h"
using namespace std;

class Line;
class Train;
class TimeController;

static class TrainManager
{
private:
	shared_ptr<Schedule> schedule;
	shared_ptr<TimeController> time;

	struct TrainState
	{
		shared_ptr<Train> train;
		size_t nextEventIndex = 0;
		string currentDay;
	};

	map<string, TrainState> managed;
public:
	TrainManager(shared_ptr<Schedule> sch, shared_ptr<TimeController> tc)
		: schedule(sch), time(tc) {
	}

	void attachTrain(shared_ptr<Train> train);
};

class Metro
{
private:
	vector<shared_ptr<Line>> lines;
	shared_ptr<TimeController> timeController;
	shared_ptr<TrainManager> manager;
public:
	Metro(shared_ptr<TimeController> tc, shared_ptr<TrainManager> trainManager) : timeController(tc), manager(trainManager) {}
	void addLine(shared_ptr<Line> line);
	void simulate(int periodSeconds, int stepSeconds);
	//void printSummary() const;
	void loadLines(const string& fileName);
};
