#include "TimeController.h"
#include <fstream>
#include <sstream>
#include <iostream>

void TimeController::advance()
{
	currentTime += step;
}

int TimeController::getCurrent() const { return currentTime; }

string TimeController::getFormattedTime() const
{
	vector<string> days = { "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье", };
	string temp = "";
	int tempCurrentTime = currentTime, seconds, minutes, hours, dayNumber;
	seconds = tempCurrentTime % 60;
	tempCurrentTime /= 60;
	minutes = tempCurrentTime % 60;
	tempCurrentTime /= 60;
	hours = tempCurrentTime % 24;
	tempCurrentTime /= 24;
	dayNumber = tempCurrentTime % 7;
	temp = (hours / 10 > 0 ? "" : "0") + to_string(hours) + ":"
		+ (minutes / 10 > 0 ? "" : "0") + to_string(minutes) + ":"
		+ (seconds / 10 > 0 ? "" : "0") + to_string(seconds) + " "
		+ days[dayNumber];
	return temp;
}

void Schedule::loadSchedule(const string& file)
{
    ifstream in(file);
    if (!in.is_open())
        throw runtime_error("Не удалось открыть файл Schedule.txt");

    string line;
    string currentDay;
    Entry currentEntry;
    bool insideDay = false;

    while (getline(in, line))
    {
        size_t start = line.find_first_not_of(" \t\r\n");
        line = (start == string::npos) ? "" : line.substr(start);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == "MONDAY" || line == "TUESDAY" ||
            line == "WEDNESDAY" || line == "THURSDAY" ||
            line == "FRIDAY" || line == "SATURDAY" ||
            line == "SUNDAY")
        {
            currentDay = line;
            currentEntry.timetable.clear();
            insideDay = true;
            continue;
        }

        if (line == "ENDDAY")
        {
            if (insideDay && !currentDay.empty())
                data[currentDay].push_back(currentEntry);

            insideDay = false;
            currentDay.clear();
            continue;
        }

        if (insideDay)
        {
            istringstream ss(line);
            string station;
            int travel;

            ss >> station >> travel;

            Entry::Node node;
            node.station = station;
            node.travelTime = travel;
            currentEntry.timetable.push_back(node);
        }
    }

    cout << "Упрощённое расписание успешно загружено" << endl;
}

int Schedule::parseTime(const string& txt) const
{
    if (txt.empty()) return 0;
    int h = stoi(txt.substr(0, 2));
    int m = stoi(txt.substr(3, 2));
    int s = stoi(txt.substr(6, 2));
    return h * 3600 + m * 60 + s;
}
