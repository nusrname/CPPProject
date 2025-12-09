#include "QtUI.h"
#include "./ui_QtUI.h"

#include "../../vs/Metro.h"
#include "../../vs/Line.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setMetro(std::shared_ptr<Metro> m)
{
    metro = m;
    rebuildScene();
    update();   // перерисовать
}

void Widget::rebuildScene()
{
    drawLines.clear();
    drawStations.clear();
    drawTrains.clear();

    if (!metro) return;

    auto &lines = metro->getLines();

    // Простая раскладка: каждая линия — горизонтальная на разной высоте
    int y = 100;
    for (const auto &line : lines)
    {
        DrawLine dl;

        int x = 100;
        for (const auto &st : line->getStations())
        {
            DrawStation ds;
            ds.name = QString::fromStdString(st->getName());
            ds.pos = QPointF(x, y);

            drawStations.push_back(ds);
            dl.points.push_back(ds.pos);

            x += 150; // расстояние между станциями
        }

        drawLines.push_back(dl);
        y += 200;   // следующая линия ниже
    }

    // Поезда пока не отображаются (нет координат)
    // Позже добавим интерполяцию позиций
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Линии
    p.setPen(QPen(Qt::black, 4));
    for (const auto &line : drawLines)
    {
        for (int i = 0; i < line.points.size() - 1; ++i)
            p.drawLine(line.points[i], line.points[i + 1]);
    }

    // Станции — круги
    p.setBrush(Qt::white);
    p.setPen(QPen(Qt::blue, 2));
    for (const auto &st : drawStations)
    {
        p.drawEllipse(st.pos, 12, 12);
        p.drawText(st.pos + QPointF(15, 5), st.name);
    }

    // Поезда — треугольники (пока нет позиций, появится позже)
    p.setBrush(Qt::red);
    for (const auto &t : drawTrains)
    {
        QPointF a = t.pos;
        QPointF b = t.pos + QPointF(-10, 20);
        QPointF c = t.pos + QPointF(10, 20);

        p.drawPolygon(QPolygonF() << a << b << c);
    }
}
