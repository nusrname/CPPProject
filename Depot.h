#pragma once
#include <iostream>
#include "Line.h"
#include "Train.h"
using namespace std;

class Depot : protected Waypoint
{
private:
	vector<shared_ptr<Train>> storedTrains;
public:
	shared_ptr<Train> releaseTrain();
	void storeTrain(shared_ptr<Train> train);
};