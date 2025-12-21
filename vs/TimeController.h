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

    // 1–5% базовый шанс
    uniform_real_distribution<double> baseChance{ 0.01, 0.05 };
    uniform_real_distribution<double> roll{ 0.0, 1.0 };

    // Задержка строго 20–60 секунд
    uniform_int_distribution<int> delayDist{ 20, 60 };

    bool isRushHour(int secondsFromWeek) const
    {
        int daySeconds = secondsFromWeek % 86400;
        int hour = daySeconds / 3600;

        // 7–10 и 17–20 — часы пик
        return (hour >= 7 && hour < 10) ||
               (hour >= 17 && hour < 20);
    }

    bool isLowTrafficHour(int secondsFromWeek) const
    {
        int daySeconds = secondsFromWeek % 86400;
        int hour = daySeconds / 3600;

        // Минимальный трафик днём
        return hour >= 11 && hour < 16;
    }

    double dayMultiplier(int secondsFromWeek) const
    {
        int day = (secondsFromWeek / 86400) % 7;

        // Пример:
        // 0 — Пн, 4 — Пт → проблемные дни
        if (day == 0 || day == 4)
            return 1.5;

        // Сб, Вс — спокойнее
        if (day == 5 || day == 6)
            return 0.7;

        return 1.0;
    }

public:

    double getChance(int currentTime)
    {
        double chance = baseChance(rng);

        if (isRushHour(currentTime))
            chance *= 1.8;
        else if (isLowTrafficHour(currentTime))
            chance *= 0.6;

        chance *= dayMultiplier(currentTime);
        return chance * 100;
    }

    bool isDelayEvent(int currentTime)
    {
        double chance = baseChance(rng);

        if (isRushHour(currentTime))
            chance *= 1.8;
        else if (isLowTrafficHour(currentTime))
            chance *= 0.6;

        chance *= dayMultiplier(currentTime);

        return roll(rng) < chance;
    }

    int getRandomDelay()
    {
        return delayDist(rng);
    }
};

