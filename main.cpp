#include "Line.h"
#include "Metro.h"
#include "TimeController.h"
#include "ConsoleUI.h"
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time, step;
	try
	{
		cin >> time >> step;
		auto timeController = make_shared<TimeController>(0, step);
		auto trainManager = make_shared<TrainManager>();
		Schedule schedule;
		auto lineSchedule = schedule.get();
		Metro metro(timeController, trainManager);
		metro.loadLines("MetroData.txt");
		metro.simulate(time, step);
	}
	catch (exception ex)
	{
		cerr << ex.what();
	}
}