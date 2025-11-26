#include "Metro.h"
#include "Line.h"
#include "TimeController.h"

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
        throw runtime_error("Не удалось открыть файл MetroData");

    string line;

    shared_ptr<Line> currentLine = nullptr;

    while (getline(in, line))
    {
        if (line.empty()) continue;

        istringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "LINE")
        {
            string name;
            ss >> name;
            currentLine = make_shared<Line>(name);
            continue;
        }

        if (cmd == "STATION")
        {
            string name;
            int pos;
            ss >> name >> pos;
            currentLine->addStation(make_shared<Station>(name, pos));
            continue;
        }

        if (cmd == "DEPOT")
        {
            string name;
            int pos;
            ss >> name >> pos;

            auto depot = make_shared<Depot>(name, pos);

            if (!currentLine->getStartDepot())
                currentLine->setStartDepot(depot);
            else
                currentLine->setEndDepot(depot);

            continue;
        }

        if (cmd == "TRAIN")
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
            continue;
        }

        if (cmd == "ENDLINE")
        {
            addLine(currentLine);
            currentLine = nullptr;
            continue;
        }
    }
}
