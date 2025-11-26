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

//void Metro::loadLines()
//{
//	ifstream in("Info.txt");
//	if (!in.is_open())
//		throw runtime_error("Не удалось открыть файл расписания");
//
//	string line, sd, ed, currentLineName;
//
//	while (getline(in, line))
//	{
//		if (line.empty()) continue;
//
//		// Строки формата:
//	  // LINE LINE_NAME START_DEPOT END_DEPOT
//		if (line.rfind("LINE", 0) == 0)
//		{
//			istringstream ss(line);
//			string word;
//			ss >> word >> currentLineName;
//			data[currentLineName];
//			continue;
//			// Строки формата:
//		   // STATION NAME POSITION
//
//
//		   // Строки формата:
//		   // TRAIN NAME MAX_SPEED
//			if (line.rfind("TRAIN", 0) == 0)
//			{
//				istringstream ss(line);
//				string word, trainID, timeStr, station;
//				ss >> word >> trainID >> timeStr >> station;
//
//				int t = parseTime(timeStr);
//
//				auto& vec = data[currentLineName];
//
//				auto it = find_if(vec.begin(), vec.end(),
//					[&](auto& e) { return e.trainID == trainID; });
//
//				if (it == vec.end())
//				{
//					Entry e;
//					e.trainID = trainID;
//					e.timetable.push_back({ t, station });
//					vec.push_back(e);
//				}
//				else
//					it->timetable.push_back({ t, station });
//			}
//		}
//
//		cout << "Расписание успешно загружено" << endl;
//	}
//}