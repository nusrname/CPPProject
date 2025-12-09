#include "QtUI.h"

#include <QApplication>
#include <QTranslator>

#include "../../vs/Metro.h"
#include "../../vs/TimeController.h"
#include "../../vs/Line.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // === создаём Metro ===
    auto tc = make_shared<TimeController>(0, 0);
    auto metro = make_shared<Metro>(tc);

    // создаём тестовые данные
    auto line = make_shared<Line>("Test Line");
    line->addStation(make_shared<Station>("A"));
    line->addStation(make_shared<Station>("B"));
    line->addStation(make_shared<Station>("C"));
    metro->addLine(line);

    // === создаём окно ===
    Widget w;
    w.setMetro(metro);   // <<--- подключаем
    w.show();

    return a.exec();
}
