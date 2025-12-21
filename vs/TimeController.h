#pragma once
#include <map>
#include <random>
#include <string>
#include <vector>
using namespace std;

class Line;
class Train;

struct Entry
{
	int interval = 300;
	struct Node
	{
		string station;
		int travelTime = 0;   // время движения до следующей станции
		int stopTime = 60;     // время стоянки на текущей станции
		bool operator==(const Node& other) const 
		{
			return station == other.station &&
				travelTime == other.travelTime &&
				stopTime == other.stopTime;
		}
	};

	vector<Node> timetable;
};

class TimeController 
{
private:
	int currentTime;
	int step;
public:
	TimeController(int time, int step) : currentTime(time), step(step) {}
	void advance();
	string getFormattedTime() const;
	int getCurrent() const;
    void setCurrent(int time) {currentTime = time;}
};

class Schedule
{
private:
	map<string, vector<Entry>> data;

	void loadSchedule(const string& file);
	int parseTime(const string& txt) const;
public:
	Schedule(const string& file = "Schedule.txt") { loadSchedule(file); }

    const map<string, vector<Entry>>& get() const { return data; }
    const vector<Entry>* getTrainSchedule(const string& trainID) const
    {
        auto it = data.find(trainID);
        if (it == data.end())
            return nullptr;
        return &it->second;
    }
	const Entry& getCurrentEntry(int time) const { return data.at("MONDAY")[0]; }
};

class RandomEventGenerator
{
private:
	mt19937 rng{ random_device{}() };
    normal_distribution<double> norm{ 60, 20 }; // средняя задержка 90 сек
	uniform_real_distribution<double> chance{ 0.0, 1.0 };

public:

	bool isDelayEvent(int currentTime)
	{
        return chance(rng) < 0.03; // 3% шанс задержки на станции
	}

	int getRandomDelay()
    {
        int d = static_cast<int>(norm(rng));
        d = max(10, d);            // минимум 10 сек
        return min(d, 120);        // максимум 2 минуты
	}
};
