#include "Line.h"

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;

	for (auto station : stations)
		station->printStatus();

	cout << "\tПоездов: "
		<< activeTrains.size()
		<< " (в депо: "
		<< depotStart->getTrains().size()
		<< " / "
		<< depotEnd->getTrains().size()
		<< ")\n"
		<< "\n\tА именно: " << endl;
	for (auto train : depotStart->getTrains())
		cout << train->getID();
	for (auto& train : activeTrains)
		cout << train->getID() << endl;
}

void Line::update(int currentTime)
{
	for (auto& t : activeTrains)
		t->moveStep(currentTime);
	if (depotStart->getTrains().size() > 0)
	{
		auto tr = depotStart->getTrains().front();
		if (tr != nullptr)
			tr->moveStep(currentTime);
	}
	if (depotEnd->getTrains().size() > 0)
	{
		auto tr = depotEnd->getTrains().front();
		if (tr != nullptr)
			tr->moveStep(currentTime);
	}
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

	if (index >= 0)
		stations.at(index)->depart(train);

	int next = train->isForward() ? index++ : index--;

	if (index >= stations.size())
	{
		train->reverse();
		next = index - 1;
	}
	else if (next < 0)
	{
		train->reverse();
		next = index + 1;
	}
	stations.at(next)->arrive(train);
	return index;
}

void Station::arrive(shared_ptr<Train> train)
{
	trainsHere.push_back(train);
}

void Station::depart(shared_ptr<Train> train)
{
	trainsHere.erase(
		remove(trainsHere.begin(), trainsHere.end(), train),
		trainsHere.end()
	);
}

void Station::printStatus() const
{
	cout << "\t\tСтанция " + name + ": " << endl;
	cout << "\t\tПоездов на станции: " << trainsHere.size() << "\n\t\tА именно:" << endl;
	for (auto train : trainsHere)
		cout << "\t\t\t" << train->getID() << endl;
	cout << endl;
}

void Depot::storeTrain(shared_ptr<Train> train)
{
	storedTrains.push_back(train);
}

shared_ptr<Train> Depot::releaseTrain()
{
	auto train = storedTrains.back();
	storedTrains.erase(remove(storedTrains.begin(),
		storedTrains.end(),
		train),
		storedTrains.end());
	return train;
}

void Train::moveStep(int stepSeconds)
{
	if (depoted)
	{
		currentStationIndex = 0;
		line->startTrain(shared_from_this());
		depoted = false;
		return;
	}
	currentStationIndex = line->moveTrain(shared_from_this());
}

string Train::getID() const
{
	return id;
}