#include "ConsoleUI.h"
#include "Line.h"
#include "Metro.h"
#include "TimeController.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

void TrainManager::attachTrain(shared_ptr<Train> t)
{
	State st;
	st.train = t;

	int now = time->getCurrent();
	int day = (now / 86400) % 7;
	static const vector<string> days =
	{ "MONDAY","TUESDAY","WEDNESDAY","THURSDAY",
	 "FRIDAY","SATURDAY","SUNDAY" };

	auto& d = schedule->get();
	if (d.count(days[day]))
		st.timetable = d.at(days[day])[0].timetable;

	t->setTimetable(st.timetable);

	// запуск поездов с интервалом
	int interval = st.timetable[0].stopTime + st.timetable[0].travelTime;
	st.startTime = now + trains.size() * interval;
	trains[t->getID()] = st;
}

void TrainManager::update(int step)
{
	int now = time->getCurrent();
	//if ((now % 86400) < 21600)
	//	return;

	int day = (now / 86400) % 7;

	static const vector<string> days =
	{
		"MONDAY","TUESDAY","WEDNESDAY","THURSDAY",
		"FRIDAY","SATURDAY","SUNDAY"
	};

	auto& all = schedule->get();

	for (auto& kv : trains)
	{
		auto& st = kv.second;
		auto& t = st.train;

		if (all.count(days[day]))
		{
			auto& timetable = all.at(days[day])[0].timetable;

			// если расписание сменилось (например, настал вторник)
			if (st.timetable != timetable)
			{
				if (t->index >= 0 && t->index < t->line->getStations().size())
					t->line->getStations()[t->index]->depart(t);

				st.timetable = timetable;
				t->setTimetable(st.timetable);

				t->index = 0;
				st.index = 0;
				t->offLine = true;
				t->forward = true;
				t->stopped = true;

				t->timeLeft = st.timetable[0].stopTime;

				int interval = st.timetable[0].stopTime + st.timetable[0].travelTime;
				st.startTime = now + interval;

				st.active = false;
			}
		}

		if (!st.active)
		{
			if (now < st.startTime) continue;

			st.active = true;
			t->offLine = false;
			t->index = 0;
			st.index = 0;
			t->stopped = true;
			t->timeLeft = st.timetable[0].stopTime;

			t->line->getStations()[t->index]->arrive(t);
			continue;
		}

		if (t->offLine) continue;

		// стоянка
		if (t->stopped)
		{
			// задержки применяются только если есть достаточно времени
			if (t->timeLeft > 5)
			{
				if (randomEvents.isDelayEvent(now))
				{
					int d = randomEvents.getRandomDelay();
					t->addDelay(d);
					t->timeLeft += d;

					cout << "Задержка поезда " << t->id
						<< " на станции " << st.timetable[st.index].station
						<< " на " << d << " сек\n";
				}
			}

			t->timeLeft -= step;

			if (t->timeLeft > 0)
				continue;

			// готов к отправлению
			t->stopped = false;

			// время перегона пока не уменьшаем — обработаем после
			t->timeLeft = st.timetable[st.index].travelTime;

			// корректный depart
			if (t->index >= 0 && t->index < (int)t->line->getStations().size())
				t->line->getStations()[t->index]->depart(t);

			continue;
		}

		int baseTravel = st.timetable[st.index].travelTime;
		if (t->isDelayed())
		{
			t->setSpeedMultiplier(1.5);

			int minTravel = baseTravel / 2;
			int newTravel = max(minTravel, (int)(baseTravel / t->getSpeedMultiplier()));

			// корректная установка оставшегося времени без скачков
			t->timeLeft = min(t->timeLeft, newTravel);
		}
		else
		{
			t->setSpeedMultiplier(1.0);
		}

		// движение
		t->timeLeft -= step;
		if (t->timeLeft > 0)
			continue;

		// вычисляем следующий индекс
		int N = t->line->getStations().size();
		int nextIndex = t->forward ? t->index + 1 : t->index - 1;

		// проверка конца линии и разворот
		if (nextIndex < 0)
		{
			t->forward = true;
			nextIndex = 0;
		}
		else if (nextIndex >= N)
		{
			t->forward = false;
			nextIndex = N - 1;
		}

		// если на станции уже стоит поезд в том же направлении — задержка
		if (!t->line->getStations()[nextIndex]->canArrive(t))
		{
			t->stopped = true;
			t->timeLeft = max(60, static_cast<int>(60 + (t->line->getStations()[nextIndex]->getTrains().size() * 30)));
			t->index = t->forward ? t->index : t->index; // остаёмся на текущей станции
			continue;
		}

		t->index = nextIndex;
		t->line->getStations()[t->index]->arrive(t);

		// остановка на станции
		st.index++;
		if (st.index >= st.timetable.size())
			st.index = (int)st.timetable.size() - 1;

		t->stopped = true;
		int baseStop = st.timetable[st.index].stopTime;

		if (t->isDelayed())
		{
			// сокращение стоянки, но не менее stopTimeMin
			int newStop = max(t->getStopMin(), baseStop - t->getDelay() / 2);
			t->timeLeft = newStop;
			t->resetDelay();

			cout << "Поезд " << t->id << " сокращает стоянку до "
				<< newStop << " сек\n";
		}
		else
		{
			t->timeLeft = baseStop;
		}
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	for (int t = timeController->getCurrent(); t < periodSeconds + timeController->getCurrent(); t += stepSeconds)
	{
		ConsoleUI::ClearConsole();
		timeController->advance();
		cout << endl << timeController->getFormattedTime() << endl;
		for (auto& line : lines)
		{
			ConsoleUI::displayLineStatus(*line);
		}
		manager->update(stepSeconds);
		this_thread::sleep_for(chrono::seconds(1));
	}
}

void Metro::addLine(shared_ptr<Line> line)
{
	if (!line) return;
	if (find(lines.begin(), lines.end(), line) == lines.end())
		lines.push_back(line);
}

void Metro::loadLines(const string& fileName)
{
	ifstream in(fileName);
	if (!in.is_open())
		throw "Не удалось открыть файл MetroData.txt";

	string line;

	shared_ptr<Line> currentLine = nullptr;

	while (getline(in, line))
	{
		size_t start = line.find_first_not_of(" \t\r\n");
		line = (start == string::npos) ? "" : line.substr(start);

		if (line.empty() || line.find_first_of('#') == 0) continue;

		istringstream ss(line);
		string cmd;
		ss >> cmd;

		if (cmd == "LINE")
		{
			string name;
			ss >> name;
			currentLine = make_shared<Line>(name);
		}

		else if (cmd == "STATION")
		{
			string name;
			ss >> name;
			currentLine->addStation(make_shared<Station>(name));
		}

		else if (cmd == "TRAIN")
		{
			string id;
			ss >> id;

			auto t = make_shared<Train>(id, currentLine);
			manager->attachTrain(t);
		}

		else if (cmd == "ENDLINE")
		{
			addLine(currentLine);
			currentLine = nullptr;
		}
	}
}
