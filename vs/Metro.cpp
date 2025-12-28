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
    const string day = time->getCurrentDayName();

	auto& d = schedule->get();
    if (d.count(day))
        st.timetable = d.at(day)[0].timetable;

	t->setTimetable(st.timetable);

    int openTime = (now / 86400) * 86400 + 21600;

    if ((now % 86400) < 21600)
    {
        // если ночь — считаем от открытия метро
        st.startTime = openTime + interval;
    }
    else
    {
        st.startTime = now + interval;
    }
	st.currentOffset = schedule->getCurrentEntry(now).interval;
	trains[t->getID()] = st;
}

void TrainManager::update(int step)
{
    int now = time->getCurrent();
    // по времени работы метро
    if ((now % 86400) < 21600) return;

    const string day = time->getCurrentDayName();

    auto& all = schedule->get();
    stats.clearInterval();

	for (auto& kv : trains)
	{
		auto& st = kv.second;
		auto& t = st.train;

		// --- смена расписания (телепортация/перезапуск поезда) ---
        if (all.count(day))
		{
            auto& timetable = all.at(day)[0].timetable;
			if (st.timetable != timetable)
			{
				// если поезд всё ещё отображается на станции — удаляем его
				if (t->index >= 0 && t->index < (int)t->line->getStations().size())
					t->line->getStations()[t->index]->depart(t);

				st.timetable = timetable;
				t->setTimetable(st.timetable);

				// переводим поезд обратно в "фейковое депо" (offline)
                t->lastIndex = t->index;
                t->index = -1;
                st.index = -1;
                if (t->getID().find("01") == 1 && t->getID().size() < 3)
                {
                    t->index = 0;
                    st.index = 0;
                }
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
            t->lastIndex = t->index;
			t->index = 0;
			st.index = 0;
			t->stopped = true;

			int initialStop = 60;
			if (!st.timetable.empty())
				initialStop = st.timetable[0].stopTime;

			t->remainingStopTime = initialStop;

			// прибываем на начальную (фиктивную) станцию
			const int interval = schedule->getCurrentEntry(now).interval;
            t->line->getStations()[t->index]->arrive(t, now, interval);
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

	while (remaining > 0)
	{
		// если поезд стоит (на станции) — сначала обрабатываем стоянку
		if (t->stopped)
		{
			int consume = min(t->remainingStopTime, remaining);
			remaining -= consume;
			t->remainingStopTime -= consume;

            if (t->remainingStopTime <= 0)
            {
                // стоянка закончилась — готовимся к отправлению
                t->stopped = false;

                int travel = 0;
                if (st.index >= 0 && st.index < (int)st.timetable.size())
                    travel = st.timetable[st.index].travelTime;

                int delay = t->getDelay();

                double mult = 1.0;
                if (st.currentOffset > schedule->getCurrentEntry(time->getCurrent()).interval)
                {
                    mult = 1.5; // ускорение до ×1.5

                    // защита от ухода "вперёд расписания"
                    int maxGain = delay;
                    int reduced = travel - static_cast<int>(travel / mult);
                    if (reduced > maxGain)
                        mult = static_cast<double>(travel) / (travel - maxGain);
                }
                int realTravel = travel;
                int fastTravel = max(1, (int)ceil(travel / mult));
                int gained = realTravel - fastTravel;

                t->remainingTravelTime = fastTravel;
                t->travelTimeTotal = fastTravel;

                // КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ
                if (gained > 0)
                {
                    t->consumeDelay(gained);
                    st.currentOffset -= gained;
                }

                // удаляем поезда из станций (защита от артефактов)
                for (auto& s : t->line->getStations())
                    s->depart(t);
            }
            else break;
		}
		else
		{
			// поезд в движении (в туннеле)
			int consume = min(t->remainingTravelTime, remaining);
			remaining -= consume;
			t->remainingTravelTime -= consume;

            if (t->remainingTravelTime > 0) break;

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
			auto station = t->line->getStations()[candidate];
			int now = time->getCurrent();
/*            if (!station->canArrive(t))
            {
                t->remainingTravelTime = 1;
                remaining = 0;
				return;
			}
*/

			// Успешный заезд на станцию:
            t->lastIndex = t->index;
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
            int baseStop = st.timetable[st.index].stopTime;

            if (t->getDelay() > 0)
            {
                int delay = t->getDelay();

                int minStop = 60; // как вы и предложили
                int maxReduction = delay;

                int reduced = baseStop - minStop;
                if (reduced > maxReduction)
                    reduced = maxReduction;

                t->remainingStopTime = baseStop - reduced;
                t->remainingStopTime = max(minStop, t->remainingStopTime);

                t->consumeDelay(reduced);
            }
            else
            {
                t->remainingStopTime = baseStop;
            }

            t->stopped = true;
		}
	}
}

void Metro::addLine(shared_ptr<Line> line)
{
	if (!line) return;
	if (find(lines.begin(), lines.end(), line) == lines.end())
		lines.push_back(line);
}

int computeCycleTime(const Entry& e, int N)
{
    int sum = 0;
    for (int i = 0; i < N && i < e.timetable.size(); i++)
    {
        sum += e.timetable[i].travelTime + e.timetable[i].stopTime;
    }

    return sum * 2;
}

void Metro::generateLineFromSchedule(const string& day, int N)
{
    auto it = schedule->get().find(day);
    if (it == schedule->get().end() || it->second.empty())
        throw runtime_error("Нет записей расписания для дня " + day);

    const Entry& base = it->second.front();

    if (N < 7) N = 7;
    if (N > 20) N = 20;

    auto line = make_shared<Line>("Line-1");

    size_t stationCount = min<size_t>(base.timetable.size(), N);
    for (size_t i = 0; i < stationCount; ++i)
        line->addStation(make_shared<Station>("st" + to_string(i + 1)));

    lines.push_back(line);

    int cycleTime = computeCycleTime(base, N);
    int interval = max(1, base.interval);
    int trainCount = cycleTime / interval * 1.25;

    for (int i = 0; i < trainCount; ++i)
    {
        auto train = make_shared<Train>("T" + to_string(i + 1), line);
        manager->attachTrain(train, i * interval);
    }
}
