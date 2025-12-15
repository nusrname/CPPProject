#pragma once
#include "TimeController.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class Line;
class Train;
class TimeController;

struct SimulationStats
{
    int delayCount = 0;
    int totalDelay = 0;

    int maxInterval = 0;
    int totalIntervals = 0;
    int intervalCount = 0;

    void registerDelay(int d)
    {
        delayCount++;
        totalDelay += d;
    }

    void registerInterval(int i)
    {
        if (i <= 0) return;
        ++intervalCount;
        totalIntervals += i;
        maxInterval = max(maxInterval, i);
    }

    void print() const
    {
        cout << "\n=== Итоги моделирования ===\n"
             << "Задержек: " << delayCount << "\n"
             << "Средняя задержка: "
             << (delayCount ? totalDelay / delayCount : 0) << " сек\n"
             << "Макс. интервал: " << maxInterval << " сек\n"
             << "Средний интервал: "
             << (intervalCount ? totalIntervals / intervalCount : 0) << " сек\n";
    }
};

class TrainManager
{
private:
    shared_ptr<TimeController> time;
    shared_ptr<Schedule> schedule;
    RandomEventGenerator randomEvents;
    SimulationStats stats;

    struct State
    {
        shared_ptr<Train> train;
        vector<Entry::Node> timetable;
        bool active = false;
        size_t startTime = 0;
        int index = 0;
        int segmentTimePassed = 0;
    };

    map<string, State> trains;

public:
    TrainManager(shared_ptr<Schedule> schdule, shared_ptr<TimeController> timeController)
        : schedule(schdule), time(timeController) {
    }

    void attachTrain(shared_ptr<Train> train);
    void update(int step);
    void processMovementWithOvershoot(State& st, shared_ptr<Train>& t, int step);
    map<string, State> getTrains() { return trains; }
    void printStats() const { stats.print(); }
};

class Metro
{
private:
	vector<shared_ptr<Line>> lines;
	shared_ptr<TimeController> timeController;
	shared_ptr<TrainManager> manager;
    shared_ptr<Schedule> schedule;
public:
	Metro(shared_ptr<TimeController> tc = nullptr,
        shared_ptr<TrainManager> trainManager = nullptr) 
        : timeController(tc), manager(trainManager) {}
	
    void addLine(shared_ptr<Line> line);
    const vector<shared_ptr<Line>>& getLines() const { return lines; }

	void simulate(int periodSeconds = 0, int stepSeconds = 0);
	//void printSummary() const;
	void loadLines(const string& fileName);

    void setSchedule(shared_ptr<Schedule> s) { schedule = s; }
    shared_ptr<Schedule> getSchedule() const { return schedule; }

    //void generateSimpleLine(int stationCount, int trainCount);
    void generateLineFromSchedule(const string& day);
};
