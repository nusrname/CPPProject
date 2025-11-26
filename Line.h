#pragma once
#include <vector>
#include <memory>
#include <string>
#include <utility>
using namespace std;


class Train;

class Waypoint
{
protected:
	string name;
	int position;
	vector<shared_ptr<Train>> trains;

public:
	Waypoint(string name, int position)
		: name(move(name)), position(position) {
	}

	virtual ~Waypoint() = default;

	virtual void arrive(shared_ptr<Train> train);
	virtual void depart(shared_ptr<Train> train);

	int getPosition() const { return position; }
	const string& getName() const { return name; }
};


class Station : public Waypoint
{
private:
	int stopTimeStandard;
	int stopTimeMin;
	int lastArrivalTime = 0;
	int delayTime = 0;
	bool hasDelay = false;

public:
	Station(string name, int position, int stopTimeStandard = 120, int stopTimeMin = 30)
		: Waypoint(name, position), stopTimeStandard(stopTimeStandard), stopTimeMin(stopTimeMin) {
	}

	//void applyDelay(int seconds);

	void printStatus() const;
};


class Depot : public Waypoint, enable_shared_from_this<Depot>
{
private:
	vector<shared_ptr<Train>> stored;

public:
	Depot(string name = "depot", int position = 120)
		: Waypoint(name, position) {
	}

	void store(shared_ptr<Train> train);
	bool remove(shared_ptr<Train> train);
	shared_ptr<Train> release();

	vector<shared_ptr<Train>> getStored() const { return stored; }
};


class Line
{
private:
	string name;
	vector<shared_ptr<Station>> stations;
	shared_ptr<Depot> depotStart, depotEnd;
	vector<shared_ptr<Train>> active;

	//	shared_ptr<Schedule> schedule;
	//	int standardSegmentTime;

public:
	Line(string name = "line",
		shared_ptr<Depot> startDepot = nullptr,
		shared_ptr<Depot> endDepot = nullptr)
		: name(move(name)), depotStart(startDepot), depotEnd(endDepot) {
	}

	void addStation(shared_ptr<Station> station);
	void update(int currentTime);

	int moveTrain(shared_ptr<Train> train, int index, bool& direction);

	void startTrain(shared_ptr<Train> train, int stationIndex);
	void removeActiveTrain(const shared_ptr<Train>& train);

	void printStatus() const;
	shared_ptr<Depot> getStartDepot() const;
	shared_ptr<Depot> getEndDepot() const;
	vector<shared_ptr<Station>> getStations() const;
	void setStartDepot(shared_ptr<Depot> d) { depotStart = d; }
	void setEndDepot(shared_ptr<Depot> d) { depotEnd = d; }
};


class Train : public enable_shared_from_this<Train>
{
private:
	string id;
	shared_ptr<Line> line;
	shared_ptr<Depot> initialDepot;

	int currentStationIndex = -1;
	bool directionForward = false;
	bool depoted = true;
	bool readyToLeaveDepot = false;

	double position = 0.0;
	double speed = 0.0;
	double maxSpeed = 0.0;

	int timeToNextEvent = 0;
	bool isDelayed = false;
	int totalDelay = 0;
public:
	Train(string id, shared_ptr<Line> line, shared_ptr<Depot> depot, double maxSpeed) :
		id(move(id)), line(line), initialDepot(depot), speed(maxSpeed), maxSpeed(maxSpeed) {
	}

	void addToDepot(shared_ptr<Depot> depot);
	void moveStep(int stepSeconds);

	//void accelerateIfDelayed();
	//void adjustStopTime();
	//void reportStatus() const;

	string getID() const;
	int getCurrentStationIndex() const { return currentStationIndex; }
	bool isForward() const { return directionForward; }
	void reverse() { directionForward = !directionForward; }
};