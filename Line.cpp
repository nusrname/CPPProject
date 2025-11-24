#include "Line.h"

#pragma region LineRegion

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

void Line::startTrain(shared_ptr<Train> train, int stationIndex)
{
	if (stations.empty()) return;

	if (stationIndex < 0 || stationIndex >= stations.size())
		stationIndex = 0;

	if (std::find(activeTrains.begin(), activeTrains.end(), train) == activeTrains.end())
		activeTrains.push_back(train);
	stations.at(stationIndex)->arrive(train);
}

int Line::moveTrain(shared_ptr<Train> train, int index, bool& direction)
{
	if (stations.empty())
		return -1;

	// Уход со станции
	if (index >= 0 && index < stations.size())
		stations[index]->depart(train);

	int next = direction ? (index + 1) : (index - 1);

	// Достиг конечной — разворот
	if (next >= stations.size())
	{
		direction = false;
		next = index - 1;
	}
	else if (next < 0)
	{
		direction = true;
		next = index + 1;
	}

	if (next < 0) next = 0;
	if (next >= stations.size()) next = stations.size() - 1;

	stations.at(next)->arrive(train);
	return next;
}

shared_ptr<Depot> Line::getStartDepot() const
{
	return depotStart;
}

shared_ptr<Depot> Line::getEndDepot() const
{
	return depotEnd;
}

vector<shared_ptr<Station>> Line::getStations() const
{
	return stations;
}

#pragma endregion

#pragma region StationRegion

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

#pragma endregion

#pragma region DepotRegion

void Depot::storeTrain(shared_ptr<Train> train)
{
	storedTrains.push_back(train);
}

shared_ptr<Train> Depot::releaseTrain()
{
	if (storedTrains.empty()) return nullptr;

	auto train = storedTrains.back();
	storedTrains.pop_back();
	return train;
}

bool Depot::removeTrain(shared_ptr<Train> train)
{
	auto it = std::find(storedTrains.begin(), storedTrains.end(), train);
	if (it == storedTrains.end()) return false;
	storedTrains.erase(it);
	return true;
}

#pragma endregion

#pragma region TrainRegion

void Train::addToDepot()
{
	initialDepot->storeTrain(shared_from_this());
}

void Train::moveStep(int stepSeconds)
{
	if (depoted)  // выезд с депо
	{
		initialDepot->removeTrain(shared_from_this());
		currentStationIndex = (initialDepot == line->getStartDepot()) ? 0 : line->getStations().size() - 1;
		directionForward = (initialDepot == line->getStartDepot());
		depoted = false;
		movingToDepot = false;
		line->startTrain(shared_from_this(), currentStationIndex);
		return;
	}

	// Если поезд должен ехать в депо после конечной станции
	if (movingToDepot)
	{
		shared_ptr<Depot> targetDepot = directionForward ? line->getEndDepot() : line->getStartDepot();
		targetDepot->storeTrain(shared_from_this());
		depoted = true;
		movingToDepot = false;
		return;
	}

	int nextIndex = directionForward ? currentStationIndex + 1 : currentStationIndex - 1;

	// Достигли конечной станции — начинаем движение в депо
	if (nextIndex >= line->getStations().size())
	{
		line->getStations()[currentStationIndex]->depart(shared_from_this());
		movingToDepot = true;   // поезд едет в конечное депо
		return;
	}
	else if (nextIndex < 0)
	{
		line->getStations()[currentStationIndex]->depart(shared_from_this());
		movingToDepot = true;   // поезд едет в начальное депо
		return;
	}

	// Движение между станциями
	currentStationIndex = line->moveTrain(shared_from_this(), currentStationIndex, directionForward);
}


string Train::getID() const
{
	return id;
}

#pragma endregion
