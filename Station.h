#pragma once
#include <iostream>
using namespace std;

class Station : protected Waypoint 
{
private:
	int stoptimeStandard;
	int stopTimeMin;
	int lastArrivalTime;
	int delayTime;
	bool hasDelay;
public:
	void arrive(shared_ptr<Train> train) override;
	void depart(shared_ptr<Train> train) override;
	void applyDelay(int seconds);
	void printStatus() const;
};