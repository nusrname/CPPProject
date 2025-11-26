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
		auto tc = make_shared<TimeController>(0, step);
		Schedule schedule;
		auto lineSchedule = schedule.get();
		Metro metro(tc);
		auto sd = make_shared<Depot>("sd", 0);
		auto ed = make_shared<Depot>("ed", 60);
		for (auto line : lineSchedule)
			metro.addLine(make_shared<Line>(line.first, make_shared<Depot>("sd1", 0), make_shared<Depot>("ed2", 60)));

		auto st1 = make_shared<Station>("st1", 20);
		auto st2 = make_shared<Station>("st2", 40);
		auto l1 = make_shared<Line>("firstLine", sd, ed);

		auto tr1 = make_shared<Train>("id1", l1, sd, 10);
		auto tr2 = make_shared<Train>("id2", l1, sd, 10);

		tr1->addToDepot();
		tr2->addToDepot();
		l1->addStation(st1);
		l1->addStation(st2);

		metro.addLine(l1);
		metro.simulate(time, step);
	}
	catch (exception ex)
	{
		cerr << ex.what();
	}
}