#pragma once
#include <iostream>
#include "Line.h"
using namespace std;

class Train
{
private:
	string id;
	shared_ptr<Line> line;
	int currentStationIndex = -1;
	bool directionForward = false;
	int timeToNextEvent = 0;
	bool isDelayed = false;
	int totalDelay = 0;
public:
	Train(string id, shared_ptr<Line> line) : id(id), line(line) 
	{
		this->line->addTrainToDepot(make_shared<Train>(*this));
	}
	//void moveStep(int stepSeconds);
	//void accelerateIfDelayed();
	//void adjustStopTime();
	//void reportStatus() const;
	string getID() const;
};