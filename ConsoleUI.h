#pragma once
#include <iostream>
#include "Line.h"
#include "Metro.h"
using namespace std;

static class ConsoleUI 
{
public:
	void displayLineStatus(const Line& line);
	void displayTrainStatus(const Train& train);
	void displaySummary(const Metro& metro);
};