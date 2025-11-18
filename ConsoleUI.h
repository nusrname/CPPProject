#pragma once
#include "Line.h"

static class ConsoleUI 
{
public:
	void static displayLineStatus(const Line& line);
//	void displayTrainStatus(const Train& train);
//	void displaySummary(const Metro& metro);
	void static ClearConsole();
};