#include "Metro.h"
#include "Line.h"
#include "TimeController.h"

void TrainManager::attachTrain(shared_ptr<Train> train) 
{
	TrainState state;
	state.train = train;
	state.nextEventIndex = 0;
	managed[train->getID()] = state;
}

void TrainManager::update()
{
	int now = time->getCurrent();

	// вычисляем текущий день расписания
	int dayIndex = (now / 86400) % 7;
	static vector<string> days = {
		"MONDAY","TUESDAY","WEDNESDAY",
		"THURSDAY","FRIDAY","SATURDAY","SUNDAY"
	};
	string today = days[dayIndex];
	int secondsToday = now % 86400;

	const auto& dayData = schedule->get().at(today);

	for (pair<string, TrainState> data : managed)
	{
		string id = data.first;
		TrainState trainState = data.second;

		shared_ptr<Entry> entry = nullptr;
		for (auto& e : dayData)
			if (e.trainID == id)
			{
				entry = make_shared<Entry>(e);
				break;
			}
		if (!entry) continue;

		// если события кончились
		if (trainState.nextEventIndex >= entry->timetable.size())
			continue;

		const auto& node = entry->timetable[trainState.nextEventIndex];

		// пришло время выполнить событие
		if (node.time <= secondsToday)
		{
			switch (node.type)
			{
			case Entry::Node::DEPART:
				trainState.train->commandDepart(node.station);
				break;

			case Entry::Node::STOP:
				trainState.train->commandStop(node.station);
				break;

			case Entry::Node::ARRIVE:
				trainState.train->commandArrive(node.station);
				break;
			}

			trainState.nextEventIndex++;
		}
	}
}


void Metro::simulate(int periodSeconds, int stepSeconds)
{
	for (int t = 0; t < periodSeconds; t += stepSeconds)
	{
		ConsoleUI::ClearConsole();
		timeController->advance();
		cout << endl << timeController->getFormattedTime();
		for (auto& line : lines)
		{
			line->update(stepSeconds);
			ConsoleUI::displayLineStatus(*line);
		}
		//manager.updateAllTrains(time.getCurrent());
		this_thread::sleep_for(chrono::seconds(1));
	}
}

void Metro::addLine(shared_ptr<Line> line)
{
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
			//manager->registerTrain(t);
		}

		else if (cmd == "ENDLINE")
		{
			addLine(currentLine);
			currentLine = nullptr;
		}
	}
}
