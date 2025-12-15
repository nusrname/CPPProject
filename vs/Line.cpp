#include "Line.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>


void Line::printStatus() const
{
	cout << "\n\nЛиния " << name << ":\n";
	cout << "\tСтанций: " << stations.size() << "\n\tСписок:\n";

	for (auto& st : stations)
		st->printStatus();

	cout << endl;
}

void Line::addStation(shared_ptr<Station> station)
{
	if (find(stations.begin(), stations.end(), station) == stations.end())
		stations.push_back(station);
}

int Line::getStationIndex(const std::string& name) const
{
	for (int i = 0; i < stations.size(); ++i)
		if (stations[i]->getName() == name)
			return i;
	return -1;
}

void Station::printStatus() const
{
	constexpr char up = 24;
	constexpr char down = 25;

	cout << "\t\tСтанция " + name + ": " << endl;
	cout << "\t\tПоездов на станции: " << trains.size() << "\n\t\tА именно:" << endl;
	for (auto& train : trains)
		cout << "\t\t\t" << train->getID() << (train->isForward() ? down : up) << endl;
	cout << endl;
}

bool Station::canArrive(shared_ptr<Train> train)
{
	for (auto& tr : trains)
		if (tr == train || tr->isForward() == train->isForward())
			return false;
	return true;
}

void Station::arrive(shared_ptr<Train> train, int currentTime)
{
	if (train->isForward())
	{
		if (lastArrivalForward >= 0)
			lastInterval = currentTime - lastArrivalForward;
		lastArrivalForward = currentTime;
	}
	else
	{
		if (lastArrivalBackward >= 0)
			lastInterval = currentTime - lastArrivalBackward;
		lastArrivalBackward = currentTime;
	}
	trains.push_back(train);
}


void Station::depart(shared_ptr<Train> train)
{
	trains.erase(
		remove(trains.begin(), trains.end(), train),
		trains.end()
	);
}

bool Station::isIntervalSafe(int currentTime) const
{
	if (lastArrivalTime < 0)
		return true;
	return (currentTime - lastArrivalTime) >= 60;
}

int Station::getLastInterval() const
{
	return lastInterval;
}

