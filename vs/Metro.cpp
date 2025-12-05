#include "Metro.h"
#include "Line.h"
#include "TimeController.h"


void TrainManager::attachTrain(std::shared_ptr<Train> t)
{
	State st;
	st.train = t;

	// выбрать расписание
	int now = time->getCurrent();
	int day = (now / 86400) % 7;
	static const std::vector<std::string> days =
	{ "MONDAY","TUESDAY","WEDNESDAY","THURSDAY",
	 "FRIDAY","SATURDAY","SUNDAY" };

	auto& d = schedule->get();
	if (d.count(days[day]))
		st.timetable = d.at(days[day])[0].timetable;

	t->setTimetable(st.timetable);

	// запуск поездов с интервалом
	st.startTime = now + trains.size() * 30;
	trains[t->getID()] = st;
}

void TrainManager::update(int step)
{
	int now = time->getCurrent();

	for (auto& kv : trains)
	{
		auto& st = kv.second;
		auto t = st.train;

		if (!st.active)
		{
			if (now < st.startTime) continue;

			st.active = true;
			t->offLine = false;
			t->index = -1;
			t->schedulePos = 0;
			t->stopped = true;
			t->timeLeft = st.timetable[0].stopTime;
			continue;
		}

		// стоянка
		if (t->stopped)
		{
			t->timeLeft -= step;
			if (t->timeLeft > 0) continue;

			t->stopped = false;
			t->timeLeft = st.timetable[t->schedulePos].travelTime;
			continue;
		}

		int oldIndex = t->index;

		// движение
		t->timeLeft -= step;
		if (t->timeLeft <= 0)
		{
			// поезд уезжает со станции
			if (!t->offLine && oldIndex >= 0)
				t->line->getStations()[oldIndex]->depart(t);

			// меняем индекс
			t->index += (t->forward ? 1 : -1);
		}

		// прибываем
		t->index += (t->forward ? 1 : -1);

		int N = t->line->getStations().size();

		if (t->index < 0 || t->index >= N)
		{
			t->offLine = true;
		}
		else
		{
			t->offLine = false;
			t->line->getStations()[t->index]->arrive(t);
		}

		// остановка
		st.index++;
		if (st.index >= (int)st.timetable.size())
			st.index = (int)st.timetable.size() - 1;

		t->schedulePos = st.index;
		t->stopped = true;
		t->timeLeft = st.timetable[t->schedulePos].stopTime;
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	for (int t = 0; t < periodSeconds; t += stepSeconds)
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
		throw runtime_error("Не удалось открыть файл MetroData.txt");

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
