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
using namespace std;

void TrainManager::attachTrain(shared_ptr<Train> t, int interval)
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
	st.startTime = now + interval;
	st.currentOffset = schedule->getCurrentEntry(now).interval;
	trains[t->getID()] = st;
}

void TrainManager::update(int step)
{
	int now = time->getCurrent();
	// при желании можно снова включить ограничение по времени работы метро
	// if ((now % 86400) < 21600) return;

	int day = (now / 86400) % 7;
	const int baseInterval = schedule->getCurrentEntry(now).interval;
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
				t->remainingStopTime = st.timetable[0].stopTime;

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

			int initialStop = 60;
			if (!st.timetable.empty())
				initialStop = st.timetable[0].stopTime;

			t->remainingStopTime = initialStop;

			// прибываем на начальную (фиктивную) станцию
			const int interval = schedule->getCurrentEntry(now).interval;
			t->line->getStations()[t->index]->arrive(t, time->getCurrent(), interval);
			continue;
		}

		if (t->offLine) continue;

		// --- моделирование случайных задержек во время стоянки ---
		if (t->stopped && t->remainingStopTime > 5)
		{
			if (randomEvents.isDelayEvent(now))
			{
				int d = randomEvents.getRandomDelay();
				t->addDelay(d);
				t->remainingStopTime += d;
				st.currentOffset += d;
				stats.registerDelay(d);

				cout << "Delay: Train " << t->getID()
					<< " at station " << st.timetable[st.index].station
					<< " for " << d << " sec\n";
			}
		}

		// --- обработка движения (включая проскоки) ---
		processMovementWithOvershoot(st, t, step);
	}

	for (auto& kv : trains)
	{
		const auto& st = kv.second;
		if (!st.active) continue;

		stats.registerInterval(st.currentOffset);
	}
}

void TrainManager::processMovementWithOvershoot(State& st, shared_ptr<Train>& t, int step)
{
	// оставшееся время для обработки в этом тике
	int remaining = step;
	// время ожидания в туннеле при занятости станции
	constexpr int WAIT_WHEN_OCCUPIED = 60;

	while (remaining > 0)
	{
		// если поезд стоит (на станции) — сначала обрабатываем стоянку
		if (t->stopped)
		{
			int consume = min(t->remainingStopTime, remaining);
			remaining -= consume;
			t->remainingStopTime -= consume;

			if (t->remainingStopTime > 0) return;

			// стоянка закончилась — готовимся к отправлению
			t->stopped = false;

			int travel = 0;
			if (st.index >= 0 && st.index < (int)st.timetable.size())
				travel = st.timetable[st.index].travelTime;

			double mult = (t->getDelay() > 0) ? t->accelMultiplier : t->speedMultiplier;
			int calc = max(1, (int)ceil(travel / mult));
			t->remainingTravelTime = calc;

			// удаляем поезда из станций (защита от артефактов)
			for (auto& s : t->line->getStations())
				s->depart(t);

			// если ещё нет оставшегося времени в тике — продолжаем цикл (remaining > 0)
			if (remaining == 0) return;
		}
		else
		{
			// поезд в движении (в туннеле)
			int consume = min(t->remainingTravelTime, remaining);
			remaining -= consume;
			t->remainingTravelTime -= consume;

			if (remaining > 0) return;

			// поезд завершил перегон — нужно принять решение о заезде на следующую станцию
			int N = (int)t->line->getStations().size();
			int candidate = t->forward ? t->index + 1 : t->index - 1;

			// проверка конца линии и разворот (поезд "заезжает" в крайние фиктивные позиции)
			if (candidate < 0)
			{
				t->forward = true;
				candidate = 0;
				for (auto& s : t->line->getStations())
					s->resetArrivalForDirection(true);
			}
			else if (candidate >= N)
			{
				t->forward = false;
				candidate = N - 1;
				for (auto& s : t->line->getStations())
					s->resetArrivalForDirection(false);
			}

			// Перед попыткой arrive: убеждаемся, что мы полностью удалены из любых станций
			for (auto& s : t->line->getStations())
				s->depart(t);

			// Если нельзя заехать (станция занята поездом в том же направлении) —
			// остаёмся в туннеле и ждём небольшую паузу
			auto station = t->line->getStations()[candidate];
			int now = time->getCurrent();
			if (!station->canArrive(t) || !station->isIntervalSafe(now, t->isForward()))
			{
				// Логическая ошибка — поезд не должен заезжать на занятую станцию при корректном интервале
				// Для симуляции фиксированного интервала можно просто игнорировать этот тик
				// и зафиксировать предупреждение для отладки
				cout << "Warning: Train " << t->getID() << " attempted to arrive at occupied station "
					<< station->getName() << " — проверьте интервал." << endl;
				int waitUntil = station->getNextAllowedArrival(t->isForward());
				int delaySec = max(1, waitUntil - now);
				// поезд остаётся в туннеле и будет ждать до разрешённого времени
				t->remainingTravelTime = delaySec;
				// НЕ переводим в stopped: он всё ещё в туннеле
				return;
			}

			// Успешный заезд на станцию:
			t->index = candidate;
			const int interval = schedule->getCurrentEntry(now).interval;
			station->arrive(t, now, interval);

			if (st.currentOffset > interval)
			{
				int delta = min(OFFSET_RECOVERY_STEP,
					st.currentOffset - interval);
				st.currentOffset -= delta;
			}

			// обновляем позицию в расписании: переходим к следующей записи (если есть)
			st.index = min(st.index + 1, (int)st.timetable.size() - 1);

			// устанавливаем время стоянки на новой станции
			int baseStop = st.timetable[st.index].stopTime * t->stopMultiplier;

			if (t->getDelay() > 0)
			{
				// сокращаем стоянку (но не меньше некоторого минимума)
				int newStop = max(t->stopTimeMin, baseStop - t->getDelay() / 2);
				t->remainingStopTime = newStop;
				t->resetDelay();

				cout << "Train " << t->getID() << " shortens stop to " << newStop << " sec\n";
			}
			else
			{
				t->remainingStopTime = baseStop;
			}

			t->stopped = true;
		}
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	generateLineFromSchedule("MONDAY");
	int time = timeController->getCurrent();
	for (int t = time; t < periodSeconds + time; t += stepSeconds)
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
		//this_thread::sleep_for(chrono::seconds(1));
	}
	manager->printStats();
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
			manager->attachTrain(t, id.at(3));
		}

		else if (cmd == "ENDLINE")
		{
			addLine(currentLine);
			currentLine = nullptr;
		}
	}
}

int computeCycleTime(const Entry& e)
{
	int sum = 0;
	for (const auto& n : e.timetable)
		sum += n.travelTime + n.stopTime;

	return sum * 2;
}

void Metro::generateLineFromSchedule(const string& day)
{
	auto it = schedule->get().find(day);
	if (it == schedule->get().end() || it->second.empty())
		throw runtime_error("Нет записей расписания для дня " + day);

	const Entry& base = it->second.front();

	int cycleTime = computeCycleTime(base);
	int interval = max(1, base.interval);

	int trainCount = (cycleTime + interval - 1) / interval;

	auto line = make_shared<Line>("Line-1");

	for (size_t i = 0; i < base.timetable.size(); ++i)
		line->addStation(
			make_shared<Station>("st" + to_string(i + 1))
		);

	lines.push_back(line);

	for (int i = 0; i < trainCount; ++i)
	{
		auto train = make_shared<Train>("T" + to_string(i + 1), line);

		manager->attachTrain(train, i * interval);
	}
}
