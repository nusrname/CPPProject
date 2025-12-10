#include "QtUI.h"
#include "./ui_QtUI.h"

#include "../../vs/Metro.h"
#include "../../vs/Line.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QMessageBox>
using namespace std;

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

void Widget::setMetro(shared_ptr<Metro> m,
                      shared_ptr<Schedule> sch)
{
    metro = m;
    schedule = sch;
    rebuildScene();
    update();
}

void Widget::rebuildScene()
{
    drawLines.clear();
    drawStations.clear();
    drawTrains.clear();

    if (!metro) return;

    const auto &lines = metro->getLines();

    int y = 120;

    for (const auto &line : lines)
    {
        DrawLine dl;
        const auto &stations = line->getStations();
        if (stations.empty()) continue;

        // ищем travelTime для линии из расписания
        vector<int> travelTimes;

        if (schedule)
        {
            for (auto &pair : schedule->get())
            {
                const auto &entries = pair.second;
                if (!entries.empty())
                {
                    for (auto &nd : entries[0].timetable)
                        travelTimes.push_back(nd.travelTime);
                    break;
                }
            }
        }

        double x = 100;

        for (size_t i = 0; i < stations.size(); ++i)
        {
            DrawStation ds;
            ds.name = QString::fromStdString(stations[i]->getName());
            ds.pos = QPointF(x, y);
            ds.station = stations[i];

            drawStations.push_back(ds);
            dl.points.push_back(ds.pos);

            if (i < travelTimes.size())
                x += travelTimes[i] * PIXELS_PER_SECOND;
            else
                x += 150;
        }

        drawLines.push_back(dl);
        y += 200;
    }
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
        p.drawEllipse(st.pos, 10, 10);
        p.drawText(st.pos + QPointF(-15, 25), st.name);
    }

    // Поезда — треугольники
    p.setBrush(Qt::red);
    for (const auto &t : drawTrains)
    {
        QPointF a = t.pos;
        QPointF b = t.pos + QPointF(-10, 18);
        QPointF c = t.pos + QPointF(10, 18);

        p.drawPolygon(QPolygonF() << a << b << c);
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    QPointF click = event->pos();

    for (const auto &st : drawStations)
    {
        double dx = click.x() - st.pos.x();
        double dy = click.y() - st.pos.y();
        if (sqrt(dx*dx + dy*dy) <= 15)
        {
            auto trains = st.station->getTrains();

            QString info;
            info += "Станция: " + st.name + "\n";
                                             info += "Поездов: " + QString::number(trains.size()) + "\n";

                   for (auto &t : trains)
                   info += " - " + QString::fromStdString(t->getID()) + "\n";

            QMessageBox::information(this, "Информация", info);
        }
    }
}
