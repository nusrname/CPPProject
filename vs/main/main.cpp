#include "../Metro.h"
#include "../TimeController.h"
#include <clocale>
#include <exception>
#include <iostream>
#include <memory>
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time, step, period;
	try
	{
		cout << "Текущее время: ";
		cin >> time;
		cout << "Шаг симуляции: ";
		cin >> step;
		cout << "Время симуляции: ";
		cin >> period;

		auto timeController = make_shared<TimeController>(time, step);
		auto schedule = make_shared<Schedule>();
		auto trainManager = make_shared<TrainManager>(schedule, timeController);

		Metro metro(timeController, trainManager);
		metro.loadLines("MetroData.txt");
		metro.simulate(period, step);
	}
	catch (exception ex)
	{
		cerr << ex.what();
	}
}