#pragma once
#include <iostream>
#include <vector>
#include <string>
//#include "Line.h"
#include "TimeController.h"
//#include "TrainManager.h"
//#include "ConsoleUI.h"
//#include "Train.h"
//#include "RandomEventGenerator.h"
//#include "Schedule.h"
using namespace std;

class Metro 
{
private:
	//vector<shared_ptr<Line>> lines;
	shared_ptr<TimeController> timeController;
	//shared_ptr<TrainManager> manager;
public:
	Metro(shared_ptr<TimeController> tc): timeController(tc) {}
	//void addLine(shared_ptr<Line> line);
	//void simulate(int periodSeconds, int stepSeconds);
	//void printSummary() const;
};