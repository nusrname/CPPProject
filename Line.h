#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>
using namespace std;
class Train;
class Station;
class Depot;

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
		name(name), depotStart(startDepot), depotEnd(endDepot) {
	}
	void addStation(shared_ptr<Station> station);
	//	void dispatchTrain(bool directionForward);
	//	void update(int currentTime);
	void printStatus() const;
	void startTrain(shared_ptr<Train> train);
	void addTrainToDepot(shared_ptr<Train> train);
};

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

class Station : protected Waypoint
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

class Depot : protected Waypoint
{
private:
	vector<shared_ptr<Train>> storedTrains;
public:
	Depot(string name = "depot", int position = 120) : Waypoint(name, position) {}
	shared_ptr<Train> releaseTrain();
	void storeTrain(shared_ptr<Train> train);
	void arrive(shared_ptr<Train> train) override {}
	void depart(shared_ptr<Train> train) override {}
	vector<shared_ptr<Train>> getTrains()const { return storedTrains; }
};