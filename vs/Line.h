#pragma once
#include "TimeController.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;


class Train;

class Station
{
private:
	string name;
	vector<shared_ptr<Train>> trains;
public:
	Station(string name)
		: name(move(name)) {
	}

	void printStatus() const;

	void arrive(shared_ptr<Train> train);
	bool canArrive(shared_ptr<Train> train);
	void depart(shared_ptr<Train> train);

	const string& getName() const { return name; }
	vector<shared_ptr<Train>> getTrains() const { return trains; }
};

class Line
{
private:
	string name;
	vector<shared_ptr<Station>> stations;

public:
	Line(string name = "line")
		: name(move(name)) {
	}

	void addStation(shared_ptr<Station> station);
	void printStatus() const;
	vector<shared_ptr<Station>> getStations() const { return stations; }
	int getStationIndex(const string& name) const;
};

class Train : public enable_shared_from_this<Train>
{
private:
	string id;
	shared_ptr<Line> line;

	int index = -1;
	bool forward = true;

	vector<Entry::Node> timetable;
	int schedulePos = 0;
	int timeLeft = 0;
	bool stopped = true;
	bool offLine = true;

public:
	Train(string id, shared_ptr<Line> line) :
		id(move(id)), line(line) {
	}

	void setTimetable(vector<Entry::Node> table) { timetable = table; }

	const string& getID() const { return id; }
	shared_ptr<Line> getLine() const { return line; }

	bool isOffline() const { return offLine; }
	int getIndex() const { return index; }
	bool isForward() const { return forward; }

	friend class TrainManager;
};