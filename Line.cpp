#include "Line.h"

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;
	for (auto station : stations)
		station->printStatus();
	cout << "\tПоездов на ветке: " << activeTrains.size() << "\n\tА именно: " << endl;
	for (auto train : activeTrains)
		cout << train->getID() << endl;
}

void Line::addStation(shared_ptr<Station> station)
{
	stations.push_back(station);
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
	for (auto train : trainsHere)
		cout << "\t\t\t" << train->getID() << endl;
	cout << endl;
}