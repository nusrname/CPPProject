#include "QtUI.h"
#include "./ui_QtUI.h"

#include "../../vs/Metro.h"
#include "../../vs/Line.h"

#include <QFormLayout>
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
    setMinimumSize(800, 600);

    // --- UI: поля ввода ---
    labelParams = new QLabel("Параметры симуляции не заданы", this);
    labelParams->setGeometry(20, 20, 400, 25);

    // --- Время ---
    labelTime = new QLabel("00:00:00", this);
    labelTime->setGeometry((this->size().width() - labelTime->size().width()) / 2, 50, 200, 30);

    // таймер симуляции
    connect(&timer, &QTimer::timeout, this, &Widget::tick);

    btnPause = new QPushButton("Пауза", this);
    btnPause->setGeometry((this->size().width() - labelTime->size().width()) / 2, this->size().height() - 50, 120, 32);
    btnPause->setEnabled(false); // активируется после старта

    connect(btnPause, &QPushButton::clicked, this, &Widget::onPauseClicked);

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

    //rebuildScene();
    //update();
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
        // смещение над/под линией
        double offset = t.aboveLine ? -10 : 10;
        QPointF base = t.pos + QPointF(0, offset);

        // направление
        double angle = 0;
        if (t.forward)
            angle = atan2( t.nextPos.y() - t.pos.y(),
                          t.nextPos.x() - t.pos.x() );
        else
            angle = atan2( t.pos.y() - t.nextPos.y(),
                          t.pos.x() - t.nextPos.x() );

        p.save();
        p.translate(base);
        p.rotate(angle * 180.0 / M_PI);

        // рисуем треугольник «носом» вперёд
        QPolygonF poly;
        poly << QPointF(0, -12)
             << QPointF(-8, 8)
             << QPointF(8, 8);

        p.setBrush(Qt::red);
        p.drawPolygon(poly);
        p.restore();
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

            QString info = "Станция: " + st.name + "\n"
                           + "Интервал: \n"
                           + "Время стоянки: \n"
                           + "Время движения до станции: \n"
                           + "Поездов: " + QString::number(trains.size())
                           + "\n";

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


    DrawTrain dt;
    dt.pos = QPointF(200, 200);
    dt.nextPos = QPointF(300, 200);
    dt.forward = true;
    dt.aboveLine = false;
    drawTrains.push_back(dt);

    for (auto &kv : manager->getTrains())
    {
        const auto &st = kv.second;
        auto t = st.train;

        if (!st.active || t->isOffline())
            continue;

        int idx = t->getIndex();
        if (idx < 0 || idx >= st.timetable.size())
            continue;

        QPointF stationPos;
        bool found = false;

        // текущая станция
        const std::string &currName = st.timetable[idx].station;
        for (auto &ds : drawStations)
        {
            if (ds.name.toStdString() == currName)
            {
                stationPos = ds.pos;
                found = true;
                break;
            }
        }

        if (!found)
            continue; // не нашли станцию — не рисуем

        QPointF drawPos = stationPos;
        QPointF nextPos = stationPos;

        // если поезд движется — рисуем в середине перегона
        if (!t->isStopped())
        {
            int nextIdx = t->isForward() ? idx + 1 : idx - 1;
            if (nextIdx >= 0 && nextIdx < st.timetable.size())
            {
                const std::string &nextName = st.timetable[nextIdx].station;
                for (auto &ds : drawStations)
                {
                    if (ds.name.toStdString() == nextName)
                    {
                        nextPos = ds.pos;
                        drawPos = (stationPos + nextPos) * 0.5; // середина
                        break;
                    }
                }
            }
        }

        DrawTrain dt;
        dt.id = QString::fromStdString(t->getID());
        dt.pos = drawPos;
        dt.nextPos = nextPos;
        dt.forward = t->isForward();
        dt.aboveLine = !t->isForward(); // вверх / вниз

        drawTrains.push_back(dt);
    }
}

void Widget::onStartClicked()
{
    StartDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    int start = dlg.editStart->text().toInt();
    int step = dlg.editStep->text().toInt();
    int dur = dlg.editDuration->text().toInt();

    timeController->setTime(start);
    simStep = step;
    simDuration = dur;
    simEndTime = timeController->getCurrent() + dur;

    metro->generateLineFromSchedule("MONDAY");
    rebuildScene(); // чтобы сразу отобразить станции
    updateTrainsOnScene();
    update();
    tick();   // запускаем одиночный цикл
}

void Widget::tick()
{
    if (paused) return;
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

void Widget::applySimParams(int start, int step, int duration)
{
    simStep = step;
    simDuration = duration;

    timeController->setTime(start);
    simEndTime = start + duration;

    labelParams->setText(
        QString("Старт: %1  |  Шаг: %2  |  Длительность: %3")
            .arg(start).arg(step).arg(duration)
        );
}

void Widget::startSimulation()
{
    paused = false;
    btnPause->setText("Пауза");
    btnPause->setEnabled(true);
    timer.start(1000);
}

void Widget::onPauseClicked()
{
    paused = !paused;

    if (paused)
    {
        btnPause->setText("Продолжить");
    }
    else
    {
        btnPause->setText("Пауза");
        tick(); // возобновляем симуляцию
    }
}

StartDialog::StartDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Параметры симуляции");
    setModal(true);

    auto *layout = new QFormLayout(this);

    editStart = new QLineEdit("0");
    editStep = new QLineEdit("1");
    editDuration = new QLineEdit("3600");

    layout->addRow("Начальное время (сек):", editStart);
    layout->addRow("Шаг (сек):", editStep);
    layout->addRow("Длительность (сек):", editDuration);

    auto *btn = new QPushButton("Старт");
    layout->addRow(btn);

    connect(btn, &QPushButton::clicked, this, &QDialog::accept);
}
