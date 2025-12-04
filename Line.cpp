#include "Line.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "TimeController.h"


void Waypoint::arrive(shared_ptr<Train> train)
{
	if (!train) return;
	if (find(trains.begin(), trains.end(), train) == trains.end())
		trains.push_back(train);
}

void Waypoint::depart(shared_ptr<Train> train)
{
	if (!train) return;
	trains.erase(remove(trains.begin(), trains.end(), train), trains.end());
}

#pragma region LineRegion

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;

	for (auto station : stations)
		station->printStatus();

	cout << "\tПоездов (активных): " << active.size()
		<< "  (в депо: " << (depotStart ? depotStart->getStored().size() : 0)
		<< " / " << (depotEnd ? depotEnd->getStored().size() : 0) << ")\n"
		<< "\n\tА именно: " << endl;

	if (depotStart)
		for (const auto& train : depotStart->getStored())
			cout << train->getID() << " (в депо start)  ";
	for (const auto& train : active)
		cout << train->getID() << " (в движ.)  ";
	if (depotEnd)
		for (const auto& train : depotEnd->getStored())
			cout << train->getID() << " (в депо end)  ";
	cout << endl;
}

void Line::addStation(shared_ptr<Station> station)
{
	if (find(stations.begin(), stations.end(), station) == stations.end())
		stations.push_back(station);
}

void Line::update(int stepTime)
{
	/*for (auto& t : active)
		t->moveStep(stepTime);

	if (depotStart && !depotStart->getStored().empty())
	{
		auto tr = depotStart->getStored().front();
		if (tr) tr->moveStep(stepTime);
	}
	if (depotEnd && !depotEnd->getStored().empty())
	{
		auto tr = depotEnd->getStored().front();
		if (tr) tr->moveStep(stepTime);
	}*/
	for (auto& t : active)
		t->updateFromManager(stepTime);
}

void Line::startTrain(shared_ptr<Train> train, int stationIndex)
{
	if (stations.empty() || !train) return;

	if (stationIndex < 0 || stationIndex >= stations.size())
		return;

	if (depotStart)
		depotStart->remove(train);
	if (depotEnd)
		depotEnd->remove(train);

	if (find(active.begin(), active.end(), train) == active.end())
		active.push_back(train);

	stations.at(stationIndex)->arrive(train);
}

int Line::moveTrain(shared_ptr<Train> train, int index, bool& direction)
{
	stations[index]->depart(train);

	int next = direction ? index + 1 : index - 1;
	stations[next]->arrive(train);

	return next;
}

shared_ptr<Depot> Line::getStartDepot() const { return depotStart; }

shared_ptr<Depot> Line::getEndDepot() const { return depotEnd; }

vector<shared_ptr<Station>> Line::getStations() const { return stations; }

void Line::removeActiveTrain(const shared_ptr<Train>& train)
{
	if (!train) return;
	active.erase(remove(active.begin(), active.end(), train), active.end());
}

#pragma endregion

#pragma region StationRegion

void Station::printStatus() const
{
	constexpr char up = 24;
	constexpr char down = 25;

	cout << "\t\tСтанция " + name + ": " << endl;
	cout << "\t\tПоездов на станции: " << trains.size() << "\n\t\tА именно:" << endl;
	for (auto train : trains)
		cout << "\t\t\t" << train->getID() << (train->isForward() ? down : up) << endl;
	cout << endl;
}

#pragma endregion

#pragma region DepotRegion

void Depot::store(shared_ptr<Train> train)
{
	if (!train) return;
	if (std::find(stored.begin(), stored.end(), train) == stored.end())
		stored.push_back(train);
}

void Depot::remove(shared_ptr<Train> train)
{
	if (!train) return;
	stored.erase(std::remove(stored.begin(), stored.end(), train), stored.end());
}

shared_ptr<Train> Depot::release()
{
	if (stored.empty()) return nullptr;

	auto train = stored.back();
	stored.pop_back();
	return train;
}

#pragma endregion	

#pragma region TrainRegion

void Train::addToDepot(shared_ptr<Depot> depot)
{
	if (!depot) return;

	initialDepot = depot;
	depoted = true;
	currentStationIndex = -1;
}

void Train::activate(const vector<Entry::Node>& table)
{
	timetable = vector<Entry::Node>(table);
	scheduleIndex = 0;

	isStopped = true;
	timeLeft = 0;

	if (timetable.empty())
		return;

	// имя первой записи в расписании
	const string& firstName = timetable.at(scheduleIndex).station;

	// ищем соответствующую станцию в ветке
	auto stations = line->getStations();
	int foundIndex = -1;
	for (int i = 0; i < stations.size(); ++i)
	{
		if (stations[i]->getName() == firstName)
		{
			foundIndex = i;
			break;
		}
	}

	if (line->getStartDepot() && line->getStartDepot()->getName() == firstName)
	{
		depoted = true;
		initialDepot = line->getStartDepot();
		return;
	}

	if (line->getEndDepot() && line->getEndDepot()->getName() == firstName)
	{
		depoted = true;
		initialDepot = line->getEndDepot();
		return;
	}

	if (foundIndex >= 0)
	{
		depoted = false;
		currentStationIndex = foundIndex;
		isStopped = true;
		timeLeft = timetable.at(0).stopTime;
		line->startTrain(shared_from_this(), foundIndex);
		return;
	}

	depoted = true;
}

void Train::updateFromManager(int deltaSeconds)
{
	if (deltaSeconds <= 0) deltaSeconds = 1;

	if (isStopped)
	{
		timeLeft -= deltaSeconds;
		if (timeLeft > 0) return;

		isStopped = false;
		if (scheduleIndex < (int)timetable.size())
			timeLeft = timetable.at(scheduleIndex).travelTime;
		else
			timeLeft = 0;
		return;
	}

	double travelTotal = (scheduleIndex < (int)timetable.size()) ? timetable.at(scheduleIndex).travelTime : 1;
	if (travelTotal <= 0) travelTotal = 1;

	timeLeft -= deltaSeconds;
	if (timeLeft > 0) return;

	// прибытие
	if (currentStationIndex >= 0 && currentStationIndex < (int)line->getStations().size())
		line->getStations()[currentStationIndex]->depart(shared_from_this());

	currentStationIndex += directionForward ? 1 : -1;

	// если пришли в депо
	if (currentStationIndex < 0 || currentStationIndex >= (int)line->getStations().size())
	{
		auto depot = directionForward ? line->getEndDepot() : line->getStartDepot();
		depot->store(shared_from_this());
		line->removeActiveTrain(shared_from_this());
		depoted = true;
		return;
	}

	line->getStations()[currentStationIndex]->arrive(shared_from_this());
	scheduleIndex++;
	isStopped = true;
	if (scheduleIndex < (int)timetable.size())
		timeLeft = timetable.at(scheduleIndex).stopTime;
	else
		timeLeft = 0;
}

void Train::beginSchedule()
{
	// поезд должен начать с первой записи расписания
	if (timetable.empty()) 
	{
		isStopped = false;
		scheduleIndex = -1;
		timeLeft = 0;
		return;
	}

	scheduleIndex = 0;

	// текущее состояние в расписании
	auto& e = timetable.at(scheduleIndex);

	//// текущая станция — первая строка расписания
	//currentStation = line->findStationByName(e.station);

	// по умолчанию считаем, что поезд ПОКА стоит
	isStopped = true;

	// время стоянки (например, startDepot 30 60 ? first=30 = стоянка)
	timeLeft = e.stopTime;

	//// следующая станция
	//if (timetable->size() > 1)
	//	nextStation = line->findStationByName(timetable->at(1).station);
	//else
	//	nextStation = nullptr;
}


string Train::getID() const
{
	return id;
}
#pragma endregion