#include "TimeController.h"

int main() 
{
	setlocale(LC_ALL, "rus");
	int time, step;
	cin >> time >> step;
	TimeController tc(time, step);
	for (int i = 0; i < 10; i++)
	{
		tc.advance();
		cout << tc.getFormattedTime() << endl;
	}
}