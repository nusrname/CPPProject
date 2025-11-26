#include "Line.h"


void Waypoint::arrive(shared_ptr<Train> train)
{
	if (find(trains.begin(), trains.end(), train) == trains.end())
		trains.push_back(train);
}

void Waypoint::depart(shared_ptr<Train> train)
{
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
	cout << endl;
}

void Line::addStation(shared_ptr<Station> station)
{
	stations.push_back(station);
}

void Line::update(int stepTime)
{
	auto activeSnapshot = active;
	for (auto& t : activeSnapshot)
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

	if (std::find(active.begin(), active.end(), train) == active.end())
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
	auto it = std::find(active.begin(), active.end(), train);
	if (it != active.end())
		active.erase(it);
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
	if (std::find(stored.begin(), stored.end(), train) == stored.end())
		stored.push_back(train);
	train->addToDepot(shared_from_this());
}

bool Depot::remove(shared_ptr<Train> train)
{
	auto it = std::find(stored.begin(), stored.end(), train);
	if (it == stored.end()) return false;
	stored.erase(it);
	return true;
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

void Train::addToDepot(shared_ptr<Depot> depot = nullptr)
{
	if (depot)
		depot->store(shared_from_this());
	else
		initialDepot->store(shared_from_this());
	depoted = true;
	readyToLeaveDepot = false;
	currentStationIndex = -1;
}

void Train::moveStep(int stepSeconds)
{
	auto stations = line->getStations();

	// Поезд находится в депо: первый тик — готовность, второй — выезд
	if (depoted)
	{
		if (!readyToLeaveDepot)
		{
			readyToLeaveDepot = true;
			return; // стоим 1 тик
		}

		// второй тик — выезжаем
		readyToLeaveDepot = false;
		initialDepot->remove(shared_from_this());

		// определяем начальную позицию и направление исходя из того депо, в котором находился поезд
		if (initialDepot && initialDepot == line->getStartDepot())
		{
			currentStationIndex = 0;
			directionForward = true;
			position = initialDepot->getPosition();
		}
		else
		{
			currentStationIndex = (int(line->getStations().size()) - 1);
			directionForward = false;
			position = line->getEndDepot()->getPosition();
		}

		depoted = false;

		// добавляем в активные поезда (если ещё нет) и помещаем на станцию
		line->startTrain(shared_from_this(), currentStationIndex);
		return;
	}


	// --- Определяем следующую станцию ---
	int nextIndex = directionForward ? currentStationIndex + 1
		: currentStationIndex - 1;

	// --- Если следующей станции нет: движемся в депо ---
	if (nextIndex < 0 || nextIndex >= stations.size())
	{
		shared_ptr<Depot> targetDepot =
			directionForward ? line->getEndDepot() : line->getStartDepot();

		// позиция целевого депо
		double depotPos = targetDepot->getPosition();
		double delta = speed * stepSeconds * (directionForward ? 1 : -1);
		double nextPos = position + delta;

		// достигли депо
		if ((directionForward && nextPos >= depotPos) ||
			(!directionForward && nextPos <= depotPos))
		{
			position = depotPos;

			// снятие со станции (если стояли на последней)
			if (currentStationIndex >= 0 && currentStationIndex < stations.size())
				stations[currentStationIndex]->depart(shared_from_this());

			targetDepot->store(shared_from_this());
			initialDepot = targetDepot;

			// снимаем с линии
			line->removeActiveTrain(shared_from_this());

			depoted = true;
			currentStationIndex = -1;
			readyToLeaveDepot = false;
			return;
		}

		// продолжаем ехать к депо
		position = nextPos;
		return;
	}

	// --- Движение до следующей станции ---
	auto nextStation = stations[nextIndex];
	double targetPos = nextStation->getPosition();

	double delta = speed * stepSeconds * (directionForward ? 1 : -1);
	double nextPos = position + delta;

	// достигли станции
	if ((directionForward && nextPos >= targetPos) ||
		(!directionForward && nextPos <= targetPos))
	{
		// снимаем со старой станции
		if (currentStationIndex >= 0)
			stations[currentStationIndex]->depart(shared_from_this());

		position = targetPos;

		nextStation->arrive(shared_from_this());
		currentStationIndex = nextIndex;

		return;
	}

	// --- Обычное движение между станциями ---
	position = nextPos;
}

string Train::getID() const
{
	return id;
}

#pragma endregion