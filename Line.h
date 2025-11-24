#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>
using namespace std;


class Train;

class Waypoint
{
protected:
	string name;
	int position;
	vector<shared_ptr<Train>> trainsHere;
public:
	Waypoint(string name, int position) : name(name), position(position) {}
	virtual void arrive(shared_ptr<Train> train) = 0;
	virtual void depart(shared_ptr<Train> train) = 0;
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
	Station(string name, int position, int stopTimeStandard = 120, int stopTimeMin = 30) : Waypoint(name, position), stopTimeStandard(stopTimeStandard), stopTimeMin(stopTimeMin) {}
	void arrive(shared_ptr<Train> train) override;
	void depart(shared_ptr<Train> train) override;
	//void applyDelay(int seconds);
	void printStatus() const;
};


class Depot : public Waypoint
{
private:
	vector<shared_ptr<Train>> storedTrains;

public:
	Depot(string name = "depot", int position = 120) : Waypoint(name, position) {}
	shared_ptr<Train> releaseTrain();

	void storeTrain(shared_ptr<Train> train);
	bool removeTrain(shared_ptr<Train> train);

	void arrive(shared_ptr<Train> train) override {}
	void depart(shared_ptr<Train> train) override {}
	vector<shared_ptr<Train>> getTrains() const { return storedTrains; }
};


class Line
{
private:
	string name;
	vector<shared_ptr<Station>> stations;
	shared_ptr<Depot> depotStart, depotEnd;
	vector<shared_ptr<Train>> activeTrains;

	//	shared_ptr<Schedule> schedule;
	//	int standardSegmentTime;

public:
	Line(string name = "line", shared_ptr<Depot> startDepot = nullptr, shared_ptr<Depot> endDepot = nullptr) :
		name(name), depotStart(startDepot), depotEnd(endDepot) {}

	void addStation(shared_ptr<Station> station);
	void update(int currentTime);
	void printStatus() const;
	void startTrain(shared_ptr<Train> train, int stationIndex);
	int moveTrain(shared_ptr<Train> train, int index, bool& direction);
	shared_ptr<Depot> getStartDepot() const;
	shared_ptr<Depot> getEndDepot() const;
	vector<shared_ptr<Station>> getStations() const;
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
	bool movingToDepot = false;

	int timeToNextEvent = 0;
	bool isDelayed = false;
	int totalDelay = 0;
public:
	Train(string id, shared_ptr<Line> line, shared_ptr<Depot> depot) : id(id), line(line), initialDepot(depot) {}

	void addToDepot();
	void moveStep(int stepSeconds);
	//void accelerateIfDelayed();
	//void adjustStopTime();
	//void reportStatus() const;
	string getID() const;
	int getCurrentStationIndex() const { return currentStationIndex; }
	bool isForward() const { return directionForward; }
	void reverse() { directionForward = !directionForward; }
};