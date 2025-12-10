#include "QtUI.h"

#include <QApplication>

#include "../../vs/Metro.h"
#include "../../vs/TimeController.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto timeController = make_shared<TimeController>(0, 1);
    auto schedule       = make_shared<Schedule>("../../vs/Schedule.txt");
    auto trainManager   = make_shared<TrainManager>(schedule, timeController);

    auto metro = make_shared<Metro>(timeController, trainManager);
    metro->loadLines("../../vs/MetroData.txt");

    Widget w;
    w.setMetro(metro, schedule);
    w.show();

    return a.exec();
}
