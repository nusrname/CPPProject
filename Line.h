#pragma once
#include <iostream>
#include <vector>
#include "Station.h"
#include "Depot.h"
#include "Train.h"
#include "Schedule.h"
using namespace std;

class Line 
{
private:
	string name;
	vector<shared_ptr<Station>> stations;
	shared_ptr<Depot> depotStart, depotEnd;
	vector<shared_ptr<Train>> activeTrains;
	Schedule schedule;
	int standardSegmentTime;
public:
	void addStation(shared_ptr<Station> station);
	void dispatchTrain(bool directionForward);
	void update(int currentTime);
	void printStatus() const;
};

class Waypoint 
{
protected:
	string name;
	int position;
	vector<shared_ptr<Train>> trainsHere;
public:
	virtual void arrive(shared_ptr<Train> train) = 0;
	virtual void depart(shared_ptr<Train> train) = 0;
};