#pragma once
#include "Metro.h"

static class ConsoleUI 
{
public:
	void displayLineStatus(const Line& line);
	void displayTrainStatus(const Train& train);
	void displaySummary(const Metro& metro);
};