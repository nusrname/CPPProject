#pragma once

#include <iostream>
#include <list>
#include <string>
using namespace std;

class TrainManager 
{
private:
	list<Train> firstLine;
	list<Train> secondLine;
	list<Train> leftDepot;
	list<Train> rightDepot;
	list<Station> stations;
	int N;
	int currentTime;

public:
	void SetN(int n) 
	{
		N = n; 
		list<Station> st(N);
		stations = st;
	}
	bool Simulation()
	{
		for (Train train : firstLine)
		{
			if (!train.Move())
				throw "ѕоезд " + to_string(train.GetID()) + " не смог двигатьс€ по первой линии!";
		}
		for (Train train : secondLine)
		{
			if (!train.Move())
				throw "ѕоезд " + to_string(train.GetID()) + " не смог двигатьс€ по второй линии!";
		}
		for (Train train : leftDepot)
		{
			if (!train.Move())
				throw "ѕоезд " + to_string(train.GetID()) + " не смог выехать из левого депо!";
		}
		for (Train train : rightDepot)
		{
			if (!train.Move())
				throw "ѕоезд " + to_string(train.GetID()) + " не смог выехать из правого депо!";
		}
	}


};

class Train
{
private:
	list<Station> waypointList;
	int currentWaypoint;
	bool inTime = true;
	static int id;
public:
	Train() 
	{
		id++;
	}
	bool Move()
	{

	}
	int GetID() { return id; }
};

class Station 
{
private:
	int arriveTime;
	int stayTime;
	Train *currentTrain;
public:
	Station(int arriveTime, int stayTime) : arriveTime(arriveTime), stayTime(stayTime), currentTrain(NULL) {}
	bool IsOccupied() { return currentTrain != NULL; }
};