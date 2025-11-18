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
			//line->update(time.getCurrent());
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