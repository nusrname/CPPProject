#include "Metro.h"
#include "TimeController.h"
using namespace std;

int main() 
{
	setlocale(LC_ALL, "rus");
	int time, step;
	cin >> time >> step;
	TimeController tc(time, step);
	Metro metro(make_shared<TimeController>(tc));
	for (int i = 0; i < 10; i++)
	{
		tc.advance();
		cout << tc.getFormattedTime() << endl;
	}
}