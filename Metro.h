#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <thread>
#include "ConsoleUI.h"
using namespace std;

class Line;
class Train;
class TimeController;

static class TrainManager
{
private:
	//vector<shared_ptr<Train>> allTrains;
	//shared_ptr<RandomEventGenerator> randomizer;
public:
	//void registerTrain(shared_ptr<Train> train);
	//void updateAllTrains(int currentTime);
	//void handleDelayEvent(shared_ptr<Train> train);
	//void enforceSafetuIntervals(Line& line);
};

class Metro
{
private:
	vector<shared_ptr<Line>> lines;
	shared_ptr<TimeController> timeController;
	shared_ptr<TrainManager> manager;
public:
	Metro(shared_ptr<TimeController> tc) : timeController(tc) {}
	void addLine(shared_ptr<Line> line);
	void simulate(int periodSeconds, int stepSeconds);
	//void printSummary() const;
};

