#include "Line.h"
#include "Metro.h"
#include "TimeController.h"
#include "ConsoleUI.h"
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time, step;
	cin >> time >> step;
	auto tc = make_shared<TimeController>(0, step);
	Metro metro(tc);
	auto sd = make_shared<Depot>("sd", 123);
	auto ed = make_shared<Depot>("ed", 124);

	auto st1 = make_shared<Station>("st1", 123);
	auto st2 = make_shared<Station>("st2", 124);
	auto l1 = make_shared<Line>("firstLine", sd, ed);

	auto tr2 = make_shared<Train>("id1", l1, sd);

	tr2->addToDepot();
	l1->addStation(st1);
	l1->addStation(st2);

	metro.addLine(l1);
	metro.simulate(time, step);
}