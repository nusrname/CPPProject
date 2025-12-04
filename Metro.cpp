#include "Metro.h"
#include "Line.h"
#include "TimeController.h"

void TrainManager::attachTrain(shared_ptr<Train> train)
{
	TrainState st;
	st.train = train;

	int now = time->getCurrent();
	int dayIndex = (now / 86400) % 7;
	static const vector<string> days = { "MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY","SUNDAY" };
	string day = days[dayIndex];

	auto& data = schedule->get();
	auto it = data.find(day);
	if (it != data.end() && !it->second.empty())
		st.timetable = it->second[0].timetable;
	else if (!data.empty())
		st.timetable = data.begin()->second[0].timetable;
	else
		st.timetable.clear();

	int index = (int)trains.size();
	const int STAGGER = 30; // секундах; можно сделать параметром
	st.startTime = now + index * STAGGER;

	trains[train->getID()] = st;
}


void TrainManager::update()
{
	int now = time->getCurrent();

	for (auto& kv : trains)
	{
		auto& st = kv.second;
		if (!st.active)
		{
			if (now < st.startTime)
				continue; // ещё не время

			// время пришло — извлекаем поезд из депо и ставим на линию
			st.active = true;

			// определяем станцию старта из timetable (найди индекс станции в линии)
			// Для простоты — ставим на первую станцию расписания, если она есть
			st.train->setTimetable(st.timetable);

			string firstName = st.timetable[0].station;
			// найди линию: у поезда есть ссылка на line
			auto tr = st.train;
			auto line = tr->getLine(); // добавь геттер getLine() в Train, если нет
			// найти индекс станции в line
			int found = -1;
			auto stations = line->getStations();
			for (int i = 0; i < (int)stations.size(); ++i)
				if (stations[i]->getName() == firstName) { found = i; break; }

			// если первая строка — имя депо, то стартовать со станции 0 (после удаления из депо)
			if (found < 0) found = 0;

			// атомарно перемещаем поезд из депо в линию
			// Line::startTrain теперь сам убирает из депо
			line->startTrain(tr, found);

			// инициализируем внутреннее состояние поезда
			tr->beginSchedule(); // добавить метод beginSchedule(), который установит timeLeft = stopTime of index 0 и т.д.


			continue;
		}

		// если уже активен — передаём шаг
		if (st.active)
			st.train->updateFromManager(/*delta=*/ time->getCurrent() - /*prev?*/ now); // лучше прокинуть step
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	for (int t = 0; t < periodSeconds; t += stepSeconds)
	{
		//ConsoleUI::ClearConsole();
		timeController->advance();
		cout << endl << timeController->getFormattedTime();
		for (auto& line : lines)
		{
			//line->update(stepSeconds);
			ConsoleUI::displayLineStatus(*line);
		}
		manager->update();
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
			int pos;
			ss >> name >> pos;
			currentLine->addStation(make_shared<Station>(name, pos));
		}

		else if (cmd == "DEPOT")
		{
			string name;
			int pos;
			ss >> name >> pos;

			auto depot = make_shared<Depot>(name, pos);

			if (!currentLine->getStartDepot())
				currentLine->setStartDepot(depot);
			else
				currentLine->setEndDepot(depot);
		}

		else if (cmd == "TRAIN")
		{
			string id, depotName;
			double maxSpeed;
			ss >> id >> maxSpeed >> depotName;

			shared_ptr<Depot> dep =
				(currentLine->getStartDepot()->getName() == depotName)
				? currentLine->getStartDepot()
				: currentLine->getEndDepot();

			auto t = make_shared<Train>(id, currentLine, dep, maxSpeed);
			t->addToDepot(dep);
			dep->store(t);
			manager->attachTrain(t);
		}

		else if (cmd == "ENDLINE")
		{
			addLine(currentLine);
			currentLine = nullptr;
		}
	}
}
