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
        throw runtime_error("Не удалось открыть файл расписания");

    string line, currentLineName;

    while (getline(in, line))
    {
        if (line.empty()) continue;

        if (line.rfind("LINE") == 0)
        {
            istringstream ss(line);
            string word;
            ss >> word >> currentLineName;
            data[currentLineName];
            continue;
        }

        // Строки формата:
        // TRAIN NAME HH:MM:SS DAY STATION
        if (line.rfind("TRAIN") == 0)
        {
            istringstream ss(line);
            string word, trainID, timeStr, station;
            ss >> word >> trainID >> timeStr >> station;

            int t = parseTime(timeStr);

            auto& vec = data[currentLineName];

            auto it = find_if(vec.begin(), vec.end(),
                [&](auto& e) { return e.trainID == trainID; });

            if (it == vec.end())
            {
                Entry e;
                e.trainID = trainID;
                e.timetable.push_back({ t, station });
                vec.push_back(e);
            }
            else
                it->timetable.push_back({ t, station });
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
    int d = stoi(txt.substr(9, 1));
    return h * 3600 + m * 60 + s + d * 3600 * 24;
}
