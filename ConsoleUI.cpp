#include "ConsoleUI.h"

void ConsoleUI::displayLineStatus(const Line& line)
{
	line.printStatus();
}

void ConsoleUI::ClearConsole()
{
	system("cls");
}