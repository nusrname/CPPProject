#pragma once
#include "TimeController.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class Line;
class Train;
class TimeController;

class TrainManager
{
private:
    shared_ptr<TimeController> time;
    shared_ptr<Schedule> schedule;
    RandomEventGenerator randomEvents;

    struct State
    {
        shared_ptr<Train> train;
        vector<Entry::Node> timetable;
        bool active = false;
        size_t startTime = 0;
        int index = 0;
    };

    map<string, State> trains;

public:
    TrainManager(shared_ptr<Schedule> schdule, shared_ptr<TimeController> timeController)
        : schedule(schdule), time(timeController) {
    }

    void attachTrain(shared_ptr<Train> train);
    void update(int step);
};

class Metro
{
private:
	vector<shared_ptr<Line>> lines;
	shared_ptr<TimeController> timeController;
	shared_ptr<TrainManager> manager;
public:
	Metro(shared_ptr<TimeController> tc = nullptr, shared_ptr<TrainManager> trainManager = nullptr) : timeController(tc), manager(trainManager) {}
	void addLine(shared_ptr<Line> line);
	void simulate(int periodSeconds = 0, int stepSeconds = 0);
	//void printSummary() const;
	void loadLines(const string& fileName);
};
