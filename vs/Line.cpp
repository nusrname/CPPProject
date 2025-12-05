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
	if (find(trains.begin(), trains.end(), train) != trains.end())
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
	cout << "Поехали из депо " << train->getID() << endl;
}

int Line::moveTrain(std::shared_ptr<Train> tr, int from, bool& forward)
{
	int next = forward ? from + 1 : from - 1;

	// выход с линии в депо
	if (next < 0)
	{
		depotStart->store(tr);
		removeActiveTrain(tr);
		return -1; // специальный код для депо
	}
	if (next >= stations.size())
	{
		depotEnd->store(tr);
		removeActiveTrain(tr);
		return stations.size(); // специальный код для депо
	}

	if (from >= 0 && from < stations.size())
		stations[from]->depart(tr);
	else if (from == -1)
		depotStart->depart(tr);
	else if (from == stations.size())
		depotEnd->depart(tr);

	if (next >= 0 && next < stations.size())
		stations[next]->arrive(tr);
	else if (next == -1)
		depotStart->arrive(tr);
	else if (next == stations.size())
		depotEnd->arrive (tr);

	return next;
}

void Line::removeActiveTrain(const shared_ptr<Train>& train)
{
	if (!train) return;
	active.erase(remove(active.begin(), active.end(), train), active.end());
}

int Line::getStationIndex(const std::string& name) const
{
	for (int i = 0; i < stations.size(); ++i)
		if (stations[i]->getName() == name)
			return i;
	return -1;
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
	int s = line->getStationIndex(timetable[0].station);
	if (s < 0)
	{
		// жёсткая синхронизация
		currentStationIndex = 0;
	}
	else
	{
		currentStationIndex = s;
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

	if (s >= 0)
	{
		depoted = false;
		isStopped = true;
		timeLeft = timetable.at(0).stopTime;
		line->startTrain(shared_from_this(), s);
		return;
	}

	depoted = true;
}

void Train::updateFromManager(int delta)
{
	if (delta <= 0) delta = 1;

	// стоянка
	if (isStopped)
	{
		timeLeft -= delta;
		if (timeLeft > 0) return;

		cout << "Стоим" << endl;
		isStopped = false;
		timeLeft = timetable[scheduleIndex].travelTime;
		return;
	}

	// движение
	timeLeft -= delta;
	if (timeLeft > 0) return;

	// приехали к следующей станции — переместить поезд
	bool dir = directionForward;
	int newIndex = line->moveTrain(shared_from_this(), currentStationIndex, dir);
	directionForward = dir;
	currentStationIndex = newIndex;

	// достигли депо
	if (newIndex < 0) 
	{
		auto d = line->getStartDepot();
		d->store(shared_from_this());
		line->removeActiveTrain(shared_from_this());
		depoted = true;
		return;
	}
	if (newIndex >= line->getStations().size()) 
	{
		auto d = line->getEndDepot();
		d->store(shared_from_this());
		line->removeActiveTrain(shared_from_this());
		depoted = true;
		return;
	}

	// следующая запись расписания
	scheduleIndex++;
	if (scheduleIndex >= timetable.size())
		scheduleIndex = (int)timetable.size() - 1;

	isStopped = true;
	timeLeft = timetable[scheduleIndex].stopTime;
	cout << "Поехали" << endl;
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