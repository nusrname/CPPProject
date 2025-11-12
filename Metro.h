#pragma once
#include <iostream>
#include <vector>
#include "Line.h"
#include "TimeController.h"
#include "TrainManager.h"
#include "ConsoleUI.h"
using namespace std;

static class Metro 
{
private:
	vector<shared_ptr<Line>> lines;
	TimeController time;
	TrainManager manager;
public:
	void addLine(shared_ptr<Line> line);
	void simulate(int periodSeconds, int stepSeconds);
	void printSummary() const;
};