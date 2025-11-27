#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <thread>
#include <fstream>
#include <sstream>
#include "ConsoleUI.h"
using namespace std;

class Line;
class Train;
class TimeController;

static class TrainManager
{
private:
	static vector<shared_ptr<Train>> allTrains;
	//shared_ptr<RandomEventGenerator> randomizer;
public:
	//void registerTrain(shared_ptr<Train> train);
	//void updateAllTrains(int currentTime);
	//void handleDelayEvent(shared_ptr<Train> train);
	//void enforceSafetyIntervals(Line& line);
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

