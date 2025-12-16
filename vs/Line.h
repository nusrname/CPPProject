#pragma once
#include "TimeController.h"
#include <memory>
#include <string>
#include <vector>


class Train;

class Station
{
private:
	string name;
	vector<shared_ptr<Train>> trains;
	
	int lastArrivalForward = -1;
	int lastArrivalBackward = -1;
	int lastIntervalValue = 0;

	static constexpr int SAFE_INTERVAL = 60;
public:
	explicit Station(string name);

	void arrive(const shared_ptr<Train>& train, int currentTime);
	void depart(const shared_ptr<Train>& train);

	bool canArrive(const shared_ptr<Train>& train) const;
	bool isIntervalSafe(int currentTime, bool forward) const;
	void resetArrivalForDirection(bool forward);

	int lastInterval() const noexcept;
	const string& getName() const noexcept;
	const vector<shared_ptr<Train>>& getTrains() const noexcept;

	void printStatus() const;
};

class Line
{
private:
	string name;
	vector<shared_ptr<Station>> stations;

public:
	explicit Line(string name);

	void addStation(const shared_ptr<Station>& station);
	int getStationIndex(const string& name) const;

	const vector<shared_ptr<Station>>& getStations() const noexcept;
	const string& getName() const noexcept;

	void printStatus() const;
};

class Train : public enable_shared_from_this<Train>
{
private:
	string id;
	shared_ptr<Line> line;

	int index = -1;
	bool forward = true;
	bool stopped = true;
	bool offLine = true;

	int delay = 0;
	int timeLeft = 0;
         
	double speedMultiplier = 1.0;
	double accelMultiplier = 1.5;
	int stopTimeMin = 60;
	double stopMultiplier = 1.0;

	vector<Entry::Node> timetable;

public:
	Train(string id, shared_ptr<Line> line);

	const string& getID() const noexcept;
	const shared_ptr<Line>& getLine() const noexcept;

	bool isForward() const noexcept;
	bool isStopped() const noexcept;
	bool isOffline() const noexcept;

	int getIndex() const noexcept;
	int getDelay() const noexcept;
	int getTimeLeft() const noexcept;

	void addDelay(int sec);
	void resetDelay();

	void setTimetable(vector<Entry::Node> table);

	friend class TrainManager;
};
