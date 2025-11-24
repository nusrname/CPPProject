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
	TimeController tc(0, step);
	ConsoleUI consoleUI;
	Metro metro(make_shared<TimeController>(tc));
	Depot sd("sd", 123), ed("ed", 124);
	Station st1("st1", 123);
	Station st2("st2", 124);
	Line l1("firstLine", make_shared<Depot>(sd), make_shared<Depot>(ed));

	Train tr2("id1", make_shared<Line>(l1));
	tr2.addToDepot();
	l1.addStation(make_shared<Station>(st1));
	l1.addStation(make_shared<Station>(st2));
	metro.addLine(make_shared<Line>(l1));
	metro.simulate(time, step);
}