#include "Line.h"
#include <algorithm>
#include <iostream>
#include <ostream>
using namespace std;


Station::Station(string name) : name(move(name)) {}

void Station::arrive(const shared_ptr<Train>& train, int time)
{
	int& lastArrival = train->isForward()
		? lastArrivalForward
		: lastArrivalBackward;

	if (lastArrival >= 0)
		lastIntervalValue = time - lastArrival;

	lastArrival = time;
	trains.push_back(train);
}

void Station::depart(const shared_ptr<Train>& train)
{
	trains.erase(
		remove(trains.begin(), trains.end(), train),
		trains.end()
	);
}

bool Station::canArrive(const shared_ptr<Train>& train) const
{
	return none_of(
		trains.begin(), trains.end(),
		[&](const auto& t)
		{
			return t == train || t->isForward() == train->isForward();
		}
	);
}

bool Station::isIntervalSafe(int currentTime) const
{
	int last = max(lastArrivalForward, lastArrivalBackward);
	return last < 0 || (currentTime - last) >= SAFE_INTERVAL;
}

void Station::resetArrivalForDirection(bool forward)
{
	(forward ? lastArrivalForward : lastArrivalBackward) = -1;
}

int Station::lastInterval() const noexcept { return lastIntervalValue; }
const string& Station::getName() const noexcept { return name; }
const vector<shared_ptr<Train>>& Station::getTrains() const noexcept { return trains; }

void Station::printStatus() const
{
	constexpr char up = 24;
	constexpr char down = 25;

	cout << "\t\tСтанция " + name + ": " << endl;
	cout << "\t\tПоездов на станции: " << trains.size() << "\n\t\tА именно:" << endl;
	for (auto& train : trains)
		cout << "\t\t\t" << train->getID() << (train->isForward() ? down : up) << endl;
	cout << endl;
}

Line::Line(string name) : name(move(name)) {}

void Line::addStation(const shared_ptr<Station>& station)
{
	if (find(stations.begin(), stations.end(), station) == stations.end())
		stations.push_back(station);
}

int Line::getStationIndex(const std::string& name) const
{
	for (int i = 0; i < stations.size(); ++i)
		if (stations[i]->getName() == name)
			return i;
	return -1;
}

const vector<shared_ptr<Station>>& Line::getStations() const noexcept { return stations; }
const string& Line::getName() const noexcept { return name; }

void Line::printStatus() const
{
	cout << "\n\nЛиния " << name << ":\n";
	cout << "\tСтанций: " << stations.size() << "\n\tСписок:\n";

	for (auto& st : stations)
		st->printStatus();

	cout << endl;
}

Train::Train(string id, shared_ptr<Line> line) : id(move(id)), line(line) {}

const string& Train::getID() const noexcept { return id; }
const shared_ptr<Line>& Train::getLine() const noexcept { return line; }

bool Train::isForward() const noexcept { return forward; }
bool Train::isStopped() const noexcept { return stopped; }
bool Train::isOffline() const noexcept { return offLine; }

int Train::getIndex() const noexcept { return index; }
int Train::getDelay() const noexcept { return delay; }

void Train::addDelay(int sec) { delay += sec; }
void Train::resetDelay() { delay = 0; }

void Train::setTimetable(vector<Entry::Node> table) { timetable = table; }