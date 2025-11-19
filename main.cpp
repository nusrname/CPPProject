#include "Metro.h"
#include "TimeController.h"
#include "Train.h"
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
	Line l1("firstLne", make_shared<Depot>(&sd), make_shared<Depot>(&ed));
	auto line = make_shared<Line>(&l1);
	Train tr2("id1", line);
	l1.addStation(make_shared<Station>(&st1));
	metro.addLine(line);
	metro.simulate(time, step);
}