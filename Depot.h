#pragma once
#include "Metro.h"

class Depot : protected Waypoint
{
private:
	vector<shared_ptr<Train>> storedTrains;
public:
	shared_ptr<Train> releaseTrain();
	void storeTrain(shared_ptr<Train> train);
};