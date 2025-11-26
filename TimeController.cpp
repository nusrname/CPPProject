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
	temp = (hours / 10 > 0 ? "" : "0") + to_string(hours) + ":" + (minutes / 10 > 0 ? "" : "0") + to_string(minutes) + ":" + (seconds / 10 > 0 ? "" : "0") + to_string(seconds) + " " + days[dayNumber];
	return temp;
}

void Schedule::loadSchedule(string fileName)
{
	ifstream in(fileName);
	if (!in.is_open())
	{
		fstream out(fileName);
		out.close();
		throw "Ошибка открытия файла";
	}
	cout << "Файл считан" << endl;
	in.close();
}