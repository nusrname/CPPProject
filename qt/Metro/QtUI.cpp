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
    setMinimumSize(1000, 400);

    labelTime = new QLabel(this);
    labelTime->setGeometry(10, 10, 300, 20);

    btnPause = new QPushButton("Пауза", this);
    btnPause->setGeometry(10, 40, 100, 30);

    connect(btnPause, &QPushButton::clicked,
            this, &Widget::onPauseClicked);

    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout,
            this, &Widget::tick);
}

Widget::~Widget() {}

void Widget::setMetro(shared_ptr<Metro> m,
                      shared_ptr<Schedule> sch,
                      shared_ptr<TrainManager> mgr,
                      shared_ptr<TimeController> time)
{
    metro = m;
    schedule = sch;
    manager = mgr;
    timeController = time;

    rebuildScene();
}

void Widget::rebuildScene()
{
    drawStations.clear();
    drawLines.clear();

    if (!metro) return;

    const auto& lines = metro->getLines();
    if (lines.empty()) return;

    cout << lines.size() << endl;
    auto line = lines.front();
    const auto& stations = line->getStations();

    const int margin = 100;
    const int y = height() / 2;
    const int count = stations.size();
    const double step = (width() - 2 * margin) / double(count - 1);

    DrawLine dl;
    for (int i = 0; i < count; ++i)
    {
        QPointF p(margin + i * step, y);
        dl.points.push_back(p);

        DrawStation ds;
        ds.name = QString::fromStdString(stations[i]->getName());
        ds.pos = p;
        ds.station = stations[i];

        drawStations.push_back(ds);
    }

    drawLines.push_back(dl);
}

void Widget::updateTrainsOnScene()
{
    drawTrains.clear();
    if (!manager) return;

    auto states = manager->getTrains();

    for (auto& [id, st] : states)
    {
        auto t = st.train;
        if (!st.active || t->isOffline()) continue;

        auto line = t->getLine();
        int idx = t->getIndex();
        bool forward = t->isForward();
        const auto& stations = t->getLine()->getStations();
        if (idx < 0 || idx >= (int)stations.size()) continue;

        QPointF p1, p2;
        DrawTrain dt;
        QPointF pos;
        if (t->isInTunnel())
        {
            int from = idx;
            int to   = forward ? idx + 1 : idx - 1;

            if (to < 0 || to >= drawStations.size())
                return;

            p1 = drawStations[from].pos;
            p2 = drawStations[to].pos;

            double k = t->getTravelProgress();
            pos = p1 + (p2 - p1) * k;
        }
        else
        {
            // поезд стоит на станции
            pos = drawStations[idx].pos;
        }

        pos.setY(pos.y() + (forward ? -20 : +20));
        dt.pos = pos;
        dt.forward = forward;
        dt.aboveLine = forward;
        dt.id = QString::fromStdString(t->getID());
        drawTrains.push_back(dt);
    }
}

void Widget::startSimulation()
{
    paused = false;
    timer.start();
}

void Widget::tick()
{
    if (paused) return;

    timeController->advance();
    manager->update(simStep);

    labelTime->setText(
        QString::fromStdString(timeController->getFormattedTime())
        );

    updateTrainsOnScene();
    update();

    if (timeController->getCurrent() >= simEndTime)
        timer.stop();
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // линия
    p.setPen(QPen(Qt::black, 2));
    for (auto& l : drawLines)
        for (int i = 1; i < l.points.size(); ++i)
            p.drawLine(l.points[i - 1], l.points[i]);

    // станции
    for (auto& s : drawStations)
    {
        p.setBrush(Qt::white);
        p.drawEllipse(s.pos, 6, 6);
        p.drawText(s.pos + QPointF(-20, -10), s.name);
    }

    // поезда
    for (auto& t : drawTrains)
    {
        p.setBrush(Qt::blue);
        p.drawEllipse(t.pos, 5, 5);
        p.drawText(t.pos + (t.forward ? QPointF(-8, -8) : QPointF(8, 8)), t.id);
    }
}

void Widget::onPauseClicked()
{
    paused = !paused;
    btnPause->setText(paused ? "Продолжить" : "Пауза");
}

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Параметры симуляции");
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *rowStart = new QHBoxLayout;
    rowStart->addWidget(new QLabel("Начальное время:"));
    editStart = new QLineEdit("21600"); // 6:00 утра
    editStart->setValidator(new QIntValidator(0, 86400, this));
    rowStart->addWidget(editStart);
    mainLayout->addLayout(rowStart);

    QHBoxLayout *rowStep = new QHBoxLayout;
    rowStep->addWidget(new QLabel("Шаг (сек):"));
    editStep = new QLineEdit("60");
    editStep->setValidator(new QIntValidator(1, 3600, this));
    rowStep->addWidget(editStep);
    mainLayout->addLayout(rowStep);

    QHBoxLayout *rowDur = new QHBoxLayout;
    rowDur->addWidget(new QLabel("Длительность (сек):"));
    editDuration = new QLineEdit("21600"); // 6 часов
    editDuration->setValidator(new QIntValidator(1, 86400, this));
    rowDur->addWidget(editDuration);
    mainLayout->addLayout(rowDur);

    QPushButton *btnOk = new QPushButton("OK");
    QPushButton *btnCancel = new QPushButton("Отмена");

    QHBoxLayout *rowBtns = new QHBoxLayout;
    rowBtns->addWidget(btnOk);
    rowBtns->addWidget(btnCancel);
    mainLayout->addLayout(rowBtns);

    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void Widget::applySimParams(int start, int step, int duration)
{
    simStep = step;
    simDuration = duration;
    simEndTime = start + duration;

    if (timeController)
        timeController->setCurrent(start);
}

void Widget::onStartClicked()
{
    startSimulation();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    // Заглушка — можно добавить взаимодействие с поездами/станциями
}
