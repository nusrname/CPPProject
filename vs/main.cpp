#include "Metro.h"
#include "TimeController.h"
#include <clocale>
#include <exception>
#include <iostream>
#include <memory>
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time = 0, step, period;
	try
	{
		cout << "“екущее врем€: ";
		cin >> time;
		cout << "Ўаг симул€ции: ";
		cin >> step;
		cout << "¬рем€ симул€ции: ";
		cin >> period;

		auto timeController = make_shared<TimeController>(time, step);
		auto schedule = make_shared<Schedule>();
		auto trainManager = make_shared<TrainManager>(schedule, timeController);
		auto& lineSchedule = schedule->get();
		Metro metro(timeController, trainManager);
		metro.loadLines("MetroData.txt");
		metro.simulate(period, step);
	}
	catch (exception ex)
	{
		cerr << ex.what();
	}
}