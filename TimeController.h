#pragma once
#include <iostream>
using namespace std;

static class TimeController 
{
private:
	int currentTime;
	int step;
public:
	void advance();
	string getFormattedTime() const;
	int getCurrent();
};