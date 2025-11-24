#include "Line.h"

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;
	for (auto station : stations)
		station->printStatus();
	cout << "\tПоездов на ветке: " << activeTrains.size() + depotStart->getTrains().size() + depotEnd->getTrains().size() << "\n\tА именно: " << endl;
	for (auto train : depotStart->getTrains())
		cout << train->getID();
	//for (auto& train : activeTrains)
	//	cout << train->getID() << endl;
}

void Line::update(int currentTime)
{
	for (auto train : depotStart->getTrains())
		train->moveStep(currentTime);
	for (auto train : depotEnd->getTrains())
		train->moveStep(currentTime);
	for (auto train : activeTrains)
		train->moveStep(currentTime);
}

void Line::addStation(shared_ptr<Station> station)
{
	stations.push_back(station);
}

void Line::startTrain(shared_ptr<Train> train)
{
	activeTrains.push_back(train);
	stations.at(0)->arrive(train);
}

void Line::addTrainToDepot(shared_ptr<Train> train)
{
	depotStart->storeTrain(train);
}

int Line::moveTrain(shared_ptr<Train> train)
{
	int index = train->getCurrentStationIndex();
	if (index >= stations.size())
	{
		depotEnd->storeTrain(train);
		activeTrains.pop_back();
		return -1;
	}

	if (index >= 0)
		stations.at(index)->depart(train);
	index++;
	stations.at(index)->arrive(train);
	return index;
}

void Station::arrive(shared_ptr<Train> train)
{
	trainsHere.push_back(train);
}

void Station::depart(shared_ptr<Train> train)
{

}

void Station::printStatus() const
{
	cout << "\t\tСтанция " + name + ": " << endl;
	cout << "\t\tПоездов на станции: " << trainsHere.size() << "\n\t\tА именно:" << endl;
	//for (auto train : trainsHere)
	//	cout << "\t\t\t" << train->getID() << endl;
	cout << endl;
}

void Depot::storeTrain(shared_ptr<Train> train)
{
	storedTrains.push_back(train);
}

shared_ptr<Train> Depot::releaseTrain()
{
	auto train = storedTrains.back();
	storedTrains.pop_back();
	return train;
}

void Train::moveStep(int stepSeconds)
{
	if (depoted)
	{
		currentStationIndex = 0;
		line->startTrain(make_shared<Train>(*this));
		depoted = false;
		return;
	}

	int index = line->moveTrain(make_shared<Train>(*this));
	if (index == -1) { depoted = true; directionForward = !directionForward; }
	currentStationIndex = index;
}

string Train::getID() const
{
	return id;
}