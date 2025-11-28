#include "TimeController.h"

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
    bool insideTrain = false;
    Entry currentEntry;

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
            data[currentDay];
        }

        else if (line == "ENDDAY")
        {
            currentDay.clear();
        }

        else if (line.rfind("TRAIN", 0) == 0)
        {
            istringstream ss(line);
            string w;
            ss >> w >> currentEntry.trainID;

            currentEntry.timetable.clear();
            insideTrain = true;
        }

        else if (line == "ENDTRAIN")
        {
            if (!currentDay.empty())
                data[currentDay].push_back(currentEntry);

            insideTrain = false;
        }

        else if (insideTrain)
        {
            // Форматы:
            // DEPART station HH:MM:SS
            // STOP station HH:MM:SS
            // ARRIVE station HH:MM:SS
            istringstream ss(line);

            string typeStr, station, timeStr;
            ss >> typeStr >> station >> timeStr;

            Entry::Node node;

            if (typeStr == "DEPART") node.type = Entry::Node::DEPART;
            else if (typeStr == "STOP") node.type = Entry::Node::STOP;
            else if (typeStr == "ARRIVE") node.type = Entry::Node::ARRIVE;
            else
                throw runtime_error("Неизвестная команда в расписании: " + typeStr);

            node.station = station;
            node.time = parseTime(timeStr);

            currentEntry.timetable.push_back(node);
        }
    }

    cout << "Расписание успешно загружено" << endl;
}


int Schedule::parseTime(const string& txt) const
{
    if (txt.empty()) return 0;
    int h = stoi(txt.substr(0, 2));
    int m = stoi(txt.substr(3, 2));
    int s = stoi(txt.substr(6, 2));
    return h * 3600 + m * 60 + s;
}
