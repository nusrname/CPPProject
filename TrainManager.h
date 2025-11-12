#pragma once
#include <iostream>
#include "RandomEventGenerator.h"
using namespace std;

static class TrainManager 
{
private:
	vector<shared_ptr<Train>> allTrains;
	RandomEventGenerator randomizer;
public:
	void registerTrain(shared_ptr<Train> train);
	void updateAllTrains(int currentTime);
	void handleDelayEvent(shared_ptr<Train> train);
	void enforceSafetuIntervals(Line& line);
};