#include "Line.h"

#pragma region LineRegion

void Line::printStatus() const
{
	cout << "\n\nВетка " + name + ":" << endl;
	cout << "\tСтанций на ветке: " << stations.size() << "\n\tА именно:" << endl;

	for (auto station : stations)
		station->printStatus();

	cout << "\tПоездов (активных): " << activeTrains.size()
		 << "  (в депо: " << (depotStart ? depotStart->getTrains().size() : 0)
		 << " / " << (depotEnd ? depotEnd->getTrains().size() : 0) << ")\n"
		 << "\n\tА именно: " << endl;

	if (depotStart)
		for (const auto& train : depotStart->getTrains())
			cout << train->getID() << " (в депо start)  ";
	for (const auto& train : activeTrains)
		cout << train->getID() << " (в движ.)  ";
	cout << endl;
}

void Line::update(int currentTime)
{
	auto activeSnapshot = activeTrains;
	for (auto& t : activeSnapshot)
		if (t) t->moveStep(currentTime);

	if (depotStart && !depotStart->getTrains().empty())
	{
		auto tr = depotStart->getTrains().front();
		if (tr) tr->moveStep(currentTime);
	}
	if (depotEnd && !depotEnd->getTrains().empty())
	{
		auto tr = depotEnd->getTrains().front();
		if (tr) tr->moveStep(currentTime);
	}
}

void Line::addStation(shared_ptr<Station> station)
{
	stations.push_back(station);
}

void Line::startTrain(shared_ptr<Train> train, int stationIndex)
{
	if (stations.empty() || !train) return;

	if (stationIndex < 0 || stationIndex >= stations.size())
		stationIndex = 0;

	if (std::find(activeTrains.begin(), activeTrains.end(), train) == activeTrains.end())
		activeTrains.push_back(train);

	stations.at(stationIndex)->arrive(train);
}

int Line::moveTrain(shared_ptr<Train> train, int index, bool& direction)
{
	if (stations.empty() || !train) return -1;

	// Уход со станции
	if (index >= 0 && index < stations.size())
		stations[index]->depart(train);

	int next = direction ? (index + 1) : (index - 1);

	if (next < 0 || next >= stations.size()) return -1;

	stations.at(next)->arrive(train);
	return next;
}

shared_ptr<Depot> Line::getStartDepot() const { return depotStart; }

shared_ptr<Depot> Line::getEndDepot() const { return depotEnd; }

vector<shared_ptr<Station>> Line::getStations() const { return stations; }

void Line::removeActiveTrain(const shared_ptr<Train>& train)
{
	if (!train) return;
	auto it = std::find(activeTrains.begin(), activeTrains.end(), train);
	if (it != activeTrains.end())
		activeTrains.erase(it);
}

#pragma endregion

#pragma region StationRegion

void Station::arrive(shared_ptr<Train> train)
{
	if (std::find(trainsHere.begin(), trainsHere.end(), train) == trainsHere.end())
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
	if (std::find(storedTrains.begin(), storedTrains.end(), train) == storedTrains.end())
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
	if (initialDepot) initialDepot->storeTrain(shared_from_this());
	depoted = true;
	readyToLeaveDepot = false;
	movingToDepot = false;
	currentStationIndex = -1;
}

void Train::moveStep(int /*stepSeconds*/)
{
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

		// удаляем из депо (если там)
		if (initialDepot)
			initialDepot->removeTrain(shared_from_this());

		// определяем начальную позицию и направление исходя из того депо, в котором находился поезд
		if (initialDepot && initialDepot == line->getStartDepot())
		{
			currentStationIndex = 0;
			directionForward = true;
		}
		else
		{
			currentStationIndex = static_cast<int>(line->getStations().size()) - 1;
			directionForward = false;
		}

		depoted = false;
		movingToDepot = false;

		// добавляем в активные поезда (если ещё нет) и помещаем на станцию
		line->startTrain(shared_from_this(), currentStationIndex);
		return;
	}

	// Поезд едет в депо (после конечной станции)
	if (movingToDepot)
	{
		// целевое депо по направлению движения
		shared_ptr<Depot> targetDepot = directionForward ? line->getEndDepot() : line->getStartDepot();

		// снимаем со станции, если стояли
		if (currentStationIndex >= 0 && currentStationIndex < static_cast<int>(line->getStations().size()))
			line->getStations()[currentStationIndex]->depart(shared_from_this());

		// кладём в депо
		if (targetDepot) targetDepot->storeTrain(shared_from_this());

		// обновляем, чтобы при следующем выезде направление было корректным
		initialDepot = targetDepot;

		// удаляем из активных поездов линии
		line->removeActiveTrain(shared_from_this());

		depoted = true;
		movingToDepot = false;
		currentStationIndex = -1;
		readyToLeaveDepot = false; // готовность снова потребуется
		return;
	}

	// Обычное движение: переход станция -> станция
	int nextIndex = directionForward ? currentStationIndex + 1 : currentStationIndex - 1;

	// если следующий индекс выходит за границы — помечаем, что едем в депо (следующий тик попадём в депо)
	if (nextIndex < 0 || nextIndex >= static_cast<int>(line->getStations().size()))
	{
		if (currentStationIndex >= 0 && currentStationIndex < static_cast<int>(line->getStations().size()))
			line->getStations()[currentStationIndex]->depart(shared_from_this());

		movingToDepot = true;
		return;
	}

	// стандартный переход
	int newIndex = line->moveTrain(shared_from_this(), currentStationIndex, directionForward);
	if (newIndex != -1) currentStationIndex = newIndex;
}

string Train::getID() const
{
	return id;
}

#pragma endregion
