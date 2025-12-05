#include "Line.h"
#include "Metro.h"
#include "TimeController.h"
#include <iostream>
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time, step;
	try
	{
		cin >> time >> step;
		auto timeController = make_shared<TimeController>(0, step);
		auto schedule = make_shared<Schedule>();
		auto trainManager = make_shared<TrainManager>(schedule, timeController);
		auto lineSchedule = schedule->get();
		Metro metro(timeController, trainManager);
		metro.loadLines("MetroData.txt");
		metro.simulate(time, step);
	}
	catch (exception ex)
	{
		cerr << ex.what();
	}
}