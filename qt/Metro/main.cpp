#include "QtUI.h"

#include <QApplication>

#include "../../vs/Metro.h"
#include "../../vs/TimeController.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();

    StartDialog dlg(&w);
    if (dlg.exec() != QDialog::Accepted)
        return 0;


    auto timeController = make_shared<TimeController>(dlg.editStart->text().toInt(), dlg.editStep->text().toInt());
    auto schedule       = make_shared<Schedule>("../../vs/Schedule.txt");
    //auto schedule       = make_shared<Schedule>("Schedule.txt");
    auto trainManager   = make_shared<TrainManager>(schedule, timeController);

    auto metro = make_shared<Metro>(timeController, trainManager);
    metro->setSchedule(schedule);
    int N = dlg.editStations->text().toInt();
    metro->generateLineFromSchedule("MONDAY", N);
    w.setMetro(metro, schedule, trainManager, timeController);

    if (w.applySimParams(
        dlg.editStart->text().toInt(),
        dlg.editStep->text().toInt(),
        dlg.editDuration->text().toInt()
    ))
        w.startSimulation();
    return a.exec();
}
