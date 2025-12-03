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
	for (auto& t : active)
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
	}
}

void Line::startTrain(shared_ptr<Train> train, int stationIndex)
{
	if (stations.empty() || !train) return;

	if (stationIndex < 0 || stationIndex >= stations.size())
		return;

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
	position = depot->getPosition();
}

void Train::moveStep(int stepSeconds)
{
	auto stations = line->getStations();
	//const auto& timetable = Schedule::getDaySchedule()[/*trainScheduleIndex*/0].timetable;

	if (depoted)
	{
		initialDepot->remove(shared_from_this());

		if (initialDepot == line->getStartDepot())
		{
			currentStationIndex = 0;
			directionForward = true;
			position = initialDepot->getPosition();
		}
		else
		{
			currentStationIndex = int(stations.size()) - 1;
			directionForward = false;
			position = initialDepot->getPosition();
		}

		depoted = false;
		scheduleIndex = 0;
		isStopped = true;
		//timeLeft = timetable[scheduleIndex].stopTime;

		line->startTrain(shared_from_this(), currentStationIndex);
		return;
	}

	if (isStopped)
	{
		timeLeft -= stepSeconds;
		if (timeLeft > 0)
			return;

		isStopped = false;

		/*if (scheduleIndex == (int)timetable.size() - 1)
		{
			double depotPos =
				directionForward ? line->getEndDepot()->getPosition()
				: line->getStartDepot()->getPosition();

			double delta = speed * stepSeconds * (directionForward ? 1 : -1);
			double nextPos = position + delta;

			if ((directionForward && nextPos >= depotPos) ||
				(!directionForward && nextPos <= depotPos))
			{
				if (currentStationIndex >= 0)
					stations[currentStationIndex]->depart(shared_from_this());

				position = depotPos;

				auto depot = directionForward ?
					line->getEndDepot() : line->getStartDepot();

				depot->store(shared_from_this());
				addToDepot(depot);

				line->removeActiveTrain(shared_from_this());
				depoted = true;
				currentStationIndex = -1;
				scheduleIndex = 0;
				return;
			}

			position = nextPos;
			return;
		}*/

		//timeLeft = timetable[scheduleIndex].travelTime;
		return;
	}

	double targetPos = stations[directionForward ? currentStationIndex + 1
		: currentStationIndex - 1]
		->getPosition();

	double delta = speed * stepSeconds * (directionForward ? 1 : -1);
	double nextPos = position + delta;

	bool reached =
		(directionForward && nextPos >= targetPos) ||
		(!directionForward && nextPos <= targetPos);

	if (!reached)
	{
		position = nextPos;
		timeLeft -= stepSeconds;
		return;
	}

	position = targetPos;

	if (currentStationIndex >= 0)
		stations[currentStationIndex]->depart(shared_from_this());

	currentStationIndex += (directionForward ? 1 : -1);

	stations[currentStationIndex]->arrive(shared_from_this());

	scheduleIndex++;
	isStopped = true;
	//timeLeft = timetable[scheduleIndex].stopTime;
}

string Train::getID() const
{
	return id;
}
#pragma endregion