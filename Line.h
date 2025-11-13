#pragma once
#include "Metro.h"

class Line 
{
private:
	string name;
	vector<shared_ptr<Station>> stations;
	shared_ptr<Depot> depotStart, depotEnd;
	vector<shared_ptr<Train>> activeTrains;
	shared_ptr<Schedule> schedule;
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