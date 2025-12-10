#include "QtUI.h"
#include "./ui_QtUI.h"

#include "../../vs/Metro.h"
#include "../../vs/Line.h"

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QMessageBox>
#include <QMouseEvent>
#include <cmath>
using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(1200, 800);

    // --- UI: поля ввода ---
    QLabel *lbl1 = new QLabel("Начальное время (сек):", this);
    lbl1->move(20, 20);

    editStartTime = new QLineEdit(this);
    editStartTime->setPlaceholderText("0");
    editStartTime->setGeometry(200, 15, 80, 25);

    QLabel *lbl2 = new QLabel("Шаг (сек):", this);
    lbl2->move(20, 55);

    editStep = new QLineEdit(this);
    editStep->setPlaceholderText("1");
    editStep->setGeometry(200, 50, 80, 25);

    QLabel *lbl3 = new QLabel("Длительность (сек):", this);
    lbl3->move(20, 90);

    editDuration = new QLineEdit(this);
    editDuration->setPlaceholderText("3600");
    editDuration->setGeometry(200, 85, 80, 25);

    // --- Старт ---
    btnStart = new QPushButton("Старт", this);
    btnStart->setGeometry(20, 130, 100, 32);
    connect(btnStart, &QPushButton::clicked, this, &Widget::onStartClicked);

    // --- Время ---
    labelTime = new QLabel("00:00:00", this);
    labelTime->setGeometry(20, 180, 200, 30);

    // таймер симуляции
    connect(&timer, &QTimer::timeout, this, &Widget::tick);
}

Widget::~Widget()
{
}

void Widget::setMetro(shared_ptr<Metro> m,
                      shared_ptr<Schedule> sch,
                      shared_ptr<TrainManager> man,
                      shared_ptr<TimeController> tc)
{
    metro = m;
    schedule = sch;
    manager = man;
    timeController = tc;

    rebuildScene();
    update();
}

void Widget::rebuildScene()
{
    drawLines.clear();
    drawStations.clear();

    if (!metro) return;

    int y = 250;
    for (auto &line : metro->getLines())
    {
        DrawLine dl;
        double x = 100;

        auto &stations = line->getStations();
        if (stations.empty()) continue;

        // получаем travelTime + stopTime из расписания
        vector<Entry::Node> timetable;
        if (schedule)
        {
            for (auto &pair : schedule->get())
            {
                if (!pair.second.empty())
                {
                    timetable = pair.second[0].timetable;
                    break;
                }
            }
        }

        for (int i = 0; i < stations.size(); i++)
        {
            DrawStation ds;
            ds.name = QString::fromStdString(stations[i]->getName());
            ds.pos = QPointF(x, y);
            ds.station = stations[i];

            drawStations.push_back(ds);
            dl.points.push_back(ds.pos);

            if (i < timetable.size())
                x += timetable[i].travelTime * PIXELS_PER_SECOND;
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
    p.setRenderHint(QPainter::Antialiasing);

    // линии
    p.setPen(QPen(Qt::black, 4));
    for (auto &l : drawLines)
        for (int i = 0; i < l.points.size() - 1; i++)
            p.drawLine(l.points[i], l.points[i+1]);

    // станции + время стоянки
    p.setBrush(Qt::white);
    p.setPen(QPen(Qt::blue, 2));
    for (auto &st : drawStations)
    {
        p.drawEllipse(st.pos, 10, 10);
        p.drawText(st.pos + QPointF(-15, 25), st.name);

        // отрисовка stopTime
        /*if (schedule)
        {
            for (auto &pair : schedule->get())
            {
                if (!pair.second.empty())
                    for (auto &node : pair.second[0].timetable)
                        if (node.station == st.name.toStdString())
                        {
                            p.drawText(st.pos + QPointF(-15, 40),
                                       QString("Стоянка: %1с").arg(node.stopTime));
                            break;
                        }
            }
        }*/
    }

    // поезда
    p.setBrush(Qt::red);
    for (auto &t : drawTrains)
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

            QString info = "Станция: " + st.name + "\n";
            info += "Поездов: " + QString::number(trains.size()) + "\n";

            for (auto &t : trains)
                info += " - " + QString::fromStdString(t->getID()) + "\n";

            QMessageBox::information(this, "Информация", info);
        }
    }
}

void Widget::updateTrainsOnScene()
{
    drawTrains.clear();

    if (!manager) return;

    for (auto &p : manager->getTrains())
    {
        auto &state = p.second;
        if (!state.active) continue;

        auto &train = state.train;

        int idx = state.index;
        auto &tt = state.timetable;

        if (idx < 0 || idx >= tt.size()) continue;

        string stName = tt[idx].station;
        string next;

        if (train->isForward())
        {
            if (idx + 1 < tt.size()) next = tt[idx + 1].station;
            else next = tt[idx].station;     // конечная
        }
        else
        {
            if (idx - 1 >= 0) next = tt[idx - 1].station;
            else next = tt[idx].station;     // конечная
        }

        QPointF A, B;
        bool foundA = false, foundB = false;

        for (auto &ds : drawStations)
        {
            if (ds.name.toStdString() == stName)
            {
                   A = ds.pos;
                   foundA = true;
            }
            if (ds.name.toStdString() == next)
            {
                   B = ds.pos;
                   foundB = true;
            }
        }

        if (!foundA) continue;
        if (!foundB) B = A;

        // определяем прогресс
        double total = tt[idx].travelTime * 1.0;
        double gone = total - state.startTime;

        double k = 0.0;

        if (!state.train->isStopped())
        {
            int travel = tt[train->isForward() ? idx : idx - 1].travelTime;
            if (travel > 0)
                   k = clamp(state.segmentTimePassed / double(travel), 0.0, 1.0);
        }

        DrawTrain dt;
        dt.id = QString::fromStdString(train->getID());
        dt.pos = A + (B - A) * k;
        dt.forward = train->isForward();

        drawTrains.push_back(dt);
    }
}

void Widget::onStartClicked()
{
    if (!timeController || !manager)
    {
        QMessageBox::warning(this, "Ошибка", "Симуляция не инициализирована.");
        return;
    }

    if (editStartTime->text().isEmpty() ||
        editStep->text().isEmpty() ||
        editDuration->text().isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены.");
        return;
    }

    timeController->setTime(editStartTime->text().toInt());
    simStep = editStep->text().toInt();
    simDuration = editDuration->text().toInt();

    simEndTime = timeController->getCurrent() + simDuration;

    timer.start(60);
}

void Widget::tick()
{
    if (timeController->getCurrent() >= simEndTime)
    {
        timer.stop();
        return;
    }

    timeController->advance();
    manager->update(simStep);

    updateTrainsOnScene();

    labelTime->setText(
        QString::fromLocal8Bit(timeController->getFormattedTime())
    );

    update();
}
