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
	// при желании можно снова включить ограничение по времени работы метро
	// if ((now % 86400) < 21600) return;

	int day = (now / 86400) % 7;
	static const vector<string> days =
	{ "MONDAY","TUESDAY","WEDNESDAY","THURSDAY",
	  "FRIDAY","SATURDAY","SUNDAY" };

	auto& all = schedule->get();

	for (auto& kv : trains)
	{
		auto& st = kv.second;
		auto& t = st.train;

		// --- смена расписания (телепортация/перезапуск поезда) ---
		if (all.count(days[day]))
		{
			auto& timetable = all.at(days[day])[0].timetable;
			if (st.timetable != timetable)
			{
				// если поезд всё ещё отображается на станции — удаляем его
				if (t->index >= 0 && t->index < (int)t->line->getStations().size())
					t->line->getStations()[t->index]->depart(t);

				st.timetable = timetable;
				t->setTimetable(st.timetable);

				// переводим поезд обратно в "фейковое депо" (offline)
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

		// --- запускаем поезд в соответствии с startTime ---
		if (!st.active)
		{
			if (now < st.startTime) continue;

			st.active = true;
			t->offLine = false;
			t->index = 0;
			st.index = 0;
			t->stopped = true;
			t->timeLeft = st.timetable[0].stopTime;

			// прибываем на начальную (фиктивную) станцию
			t->line->getStations()[t->index]->arrive(t);
			continue;
		}

		if (t->offLine) continue;

		// --- моделирование случайных задержек во время стоянки ---
		if (t->stopped && t->timeLeft > 5)
		{
			if (randomEvents.isDelayEvent(now))
			{
				int d = randomEvents.getRandomDelay();
				t->addDelay(d);
				t->timeLeft += d;

				cout << "Delay: Train " << t->getID()
					<< " at station " << st.timetable[st.index].station
					<< " for " << d << " sec\n";
			}
		}

		// --- обработка движения (включая проскоки) ---
		processMovementWithOvershoot(st, t, step);
	}
}

void TrainManager::processMovementWithOvershoot(State& st, shared_ptr<Train>& t, int step)
{
	// оставшееся время для обработки в этом тике
	int remaining = step;
	// время ожидания в туннеле при занятости станции
	constexpr int WAIT_WHEN_OCCUPIED = 5;

    bool wasStopped = t->stopped;
	while (remaining > 0)
	{
		// если поезд стоит (на станции) — сначала обрабатываем стоянку
		if (t->stopped)
		{
			int consume = min(remaining, t->timeLeft);
			t->timeLeft -= consume;
			remaining -= consume;

			if (t->timeLeft > 0)
			{
				// ещё стоим — выходим
				return;
			}

			// стоянка закончилась — готовимся к отправлению
            t->stopped = false;
            if (!wasStopped && t->timeLeft == st.timetable[st.index].travelTime)
                st.segmentTimePassed = 0;  // начало нового перегона
			// устанавливаем базовое время перегона до следующей станции
			t->timeLeft = int(st.timetable[st.index].travelTime / (t->isDelayed() ? t->accelMultiplier : t->speedMultiplier));

			// перед фактическим движением убеждаемся, что поезд удалён со станции
			// (защита от артефактов: если он всё ещё в списке станции, то удалим)
			for (auto& s : t->line->getStations())
				s->depart(t);

			// если remaining == 0 — выход, следующий тик обработает поезд в движении
			if (remaining == 0) return;
		}
		else
		{
			// поезд в движении (в туннеле)
            int consume = min(remaining, t->timeLeft);
            if (!t->stopped)
                st.segmentTimePassed += consume;
			t->timeLeft -= consume;
			remaining -= consume;

			if (t->timeLeft > 0)
			{
				// ещё в туннеле до следующей станции
				return;
			}

			// поезд завершил перегон — нужно принять решение о заезде на следующую станцию
			int N = (int)t->line->getStations().size();
			int candidate = t->forward ? t->index + 1 : t->index - 1;

			// проверка конца линии и разворот (поезд "заезжает" в крайние фиктивные позиции)
			if (candidate < 0)
			{
				t->forward = true;
				candidate = 0;
			}
			else if (candidate >= N)
			{
				t->forward = false;
				candidate = N - 1;
			}

			// Перед попыткой arrive: убеждаемся, что мы полностью удалены из любых станций
			for (auto& s : t->line->getStations())
				s->depart(t);

			// Если нельзя заехать (станция занята поездом в том же направлении) —
			// остаёмся в туннеле и ждём небольшую паузу
			if (!t->line->getStations()[candidate]->canArrive(t))
			{
				t->stopped = true; // ожидаем в туннеле (считать как "стоп" для простоты)
				t->timeLeft = WAIT_WHEN_OCCUPIED;
				// НЕ изменяем t->index и НЕ увеличиваем st.index — повторная попытка будет в следующем тике
				return;
			}

			// Успешный заезд на станцию:
			t->index = candidate;
			t->line->getStations()[t->index]->arrive(t);

			// обновляем позицию в расписании: переходим к следующей записи (если есть)
			st.index = min(st.index + 1, (int)st.timetable.size() - 1);

			// устанавливаем время стоянки на новой станции
			int baseStop = st.timetable[st.index].stopTime * t->stopMultiplier;

			if (t->isDelayed())
			{
				// сокращаем стоянку (но не меньше некоторого минимума)
				int newStop = max(t->getStopMin(), baseStop - t->getDelay() / 2);
				t->timeLeft = newStop;
				t->resetDelay();

				cout << "Train " << t->getID() << " shortens stop to " << newStop << " sec\n";
			}
			else
			{
				t->timeLeft = baseStop;
			}

			t->stopped = true;

			// цикл продолжается, если remaining > 0 (возможен проскок через ещё станции)
		}
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	for (int t = timeController->getCurrent(); t < periodSeconds + timeController->getCurrent(); t += stepSeconds)
	{
		//ConsoleUI::ClearConsole();
		timeController->advance();
		cout << endl << timeController->getFormattedTime() << endl;
		for (auto& line : lines)
		{
			//ConsoleUI::displayLineStatus(*line);
			line->printStatus();
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
