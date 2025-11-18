#include "Metro.h"
#include "TimeController.h"
using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	int time, step;
	cin >> time >> step;
	TimeController tc(0, step);
	ConsoleUI consoleUI;
	Metro metro(make_shared<TimeController>(tc));
	Line l1;
	Station st1("st1", 123);
	l1.addStation(make_shared<Station>(st1));
	metro.addLine(make_shared<Line>(l1));
	metro.simulate(time, step);
}