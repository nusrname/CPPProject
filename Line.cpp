#include "Line.h"

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;
	for (auto station : stations)
		station->printStatus();
	cout << "\tПоездов на ветке: " << activeTrains.size() + depotStart->getTrains().size() + depotEnd->getTrains().size() << "\n\tА именно: " << endl;
	//for (auto& train : activeTrains)
	//	cout << train->getID() << endl;
}

void Line::addStation(shared_ptr<Station> station)
{
	stations.push_back(station);
}

void Line::startTrain(shared_ptr<Train> train)
{
	activeTrains.push_back(train);
}

void Line::addTrainToDepot(shared_ptr<Train> train)
{
	depotStart->storeTrain(train);
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
	trainsHere.push_back(train);
}

shared_ptr<Train> Depot::releaseTrain()
{
	auto train = trainsHere.back();
	trainsHere.pop_back();
	return train;
}