#include "Line.h"

void Station::arrive(shared_ptr<Train> train) 
{
	trainsHere.push_back(train);
}

void Station::depart(shared_ptr<Train> train) 
{
	
}