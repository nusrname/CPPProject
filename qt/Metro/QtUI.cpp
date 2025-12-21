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
#include <QToolButton>
#include <cmath>
using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(1000, 450);

    labelParams = new QLabel(this);
    labelParams->setAlignment(Qt::AlignCenter);
    labelParams->setStyleSheet(
        "background:#f4f4f4;"
        "border:1px solid #aaa;"
        "font-weight:600;"
        );
    labelParams->setGeometry(200, 10, width() - 400, 30);

    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout,
            this, &Widget::tick);

    btnPause = new QPushButton("Пауза", this);
    btnPause->setGeometry(width() / 2, 40, 100, 30);

    connect(btnPause, &QPushButton::clicked,
            this, &Widget::onPauseClicked);

    btnSpeed = new QToolButton(this);
    btnSpeed->setText("Скорость x1");
    btnSpeed->setGeometry(width() / 2 - 100, 40, 100, 30);

    speedPopup = new QWidget(this, Qt::Popup);
    speedPopup->setFixedSize(180, 60);

    sliderSpeed = new QSlider(Qt::Horizontal, speedPopup);
    sliderSpeed->setRange(1, 10);
    sliderSpeed->setValue(1);
    sliderSpeed->setGeometry(10, 20, 160, 20);
    connect(btnSpeed, &QToolButton::clicked, this, [this]()
    {
        QPoint p = btnSpeed->mapToGlobal(QPoint(0, btnSpeed->height()));
        speedPopup->move(p);
        speedPopup->show();
    });
    connect(sliderSpeed, &QSlider::valueChanged, this, [this](int value)
    {
        timer.setInterval(1000 / value);
        btnSpeed->setText("Скорость x" + QString::number(value));
    });

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

    if (!metro || !schedule) return;

    const auto& lines = metro->getLines();
    if (lines.empty()) return;

    auto line = lines.front();
    const auto& stations = line->getStations();

    const Entry& entry = schedule->getCurrentEntry(timeController->getCurrent());
    const auto& tt = entry.timetable;

    const int margin = 100;
    const int y = height() / 2 + 20;

    double totalTravelTime = 0.0;
    for (const auto& n : tt)
        totalTravelTime += n.travelTime;

    if (totalTravelTime <= 0.0)
        totalTravelTime = 1.0;

    const double usableWidth = width() - 2 * margin;
    const double scale = usableWidth / totalTravelTime;

    DrawLine dl;
    double accTime = 0.0;
    for (size_t i = 0; i < stations.size(); ++i)
    {
        QPointF p(margin + accTime * scale, y);
        dl.points.push_back(p);

        DrawStation ds;
        ds.name = QString::fromStdString(stations[i]->getName());
        ds.pos = p;
        ds.station = stations[i];
        drawStations.push_back(ds);

        // прибавляем время до следующей станции
        if (i < tt.size())
            accTime += tt[i].travelTime;
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
            int from = t->getLastIndex();
            int to   = t->getIndex();

            if (from >= 0 && to >= 0 &&
                from < drawStations.size() &&
                to   < drawStations.size())
            {
                p1 = drawStations[from].pos;
                p2 = drawStations[to].pos;

                double k = t->getTravelProgress();
                pos = p1 + (p2 - p1) * k;
            }
            else
            {
                pos = drawStations[idx].pos;
            }
        }
        else
        {
            pos = drawStations[idx].pos;
        }

        pos.setY(pos.y() + (forward ? -20 : +20));
        dt.pos = pos;
        dt.forward = forward;
        dt.aboveLine = forward;
        dt.id = QString::fromStdString(t->getID());
        dt.delayed = t->getDelay() > 0;
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

    int activeTrains = manager->getTrains().size();
    /*for (const auto& [id, st] : manager->getTrains())
        if (st.active && !st.train->isOffline())
            ++activeTrains;*/

    labelParams->setText(
        QString("Время: %1 | Шаг: %2 сек | Поездов: %3 | %4")
            .arg(QString::fromStdString(timeController->getFormattedTime()))
            .arg(simStep)
            .arg(activeTrains)
            .arg(paused ? "Пауза" : "Симуляция")
        );

    updateTrainsOnScene();
    update();

    if (timeController->getCurrent() >= simEndTime)
    {
        timer.stop();

        manager->printStats();

        const SimulationStats& s = manager->getStats();

        QString report =
            "Итоги моделирования:\n\n"
            "Задержек: " + QString::number(s.delayCount) + "\n"
            "Средняя задержка: " +
                QString::number(s.delayCount ? s.totalDelay / s.delayCount : 0) + " сек\n\n"
            "Макс. интервал: " + QString::number(s.maxInterval) + " сек\n"
            "Средний интервал: " +
                QString::number(s.intervalCount ? s.totalIntervals / s.intervalCount : 0) + " сек";

        QMessageBox *box = new QMessageBox(
            QMessageBox::Information,
            "Симуляция завершена",
            report,
            QMessageBox::Ok,
            this
            );

        box->setAttribute(Qt::WA_DeleteOnClose);

        connect(box, &QMessageBox::finished, this, [] {
            QCoreApplication::quit();
        });

        box->open();
    }
}

void Widget::paintSceneBuffer()
{
    if (sceneBuffer.size() != size())
        sceneBuffer = QPixmap(size());

    sceneBuffer.fill(Qt::white); // или фон сцены

    QPainter p(&sceneBuffer);
    p.setRenderHint(QPainter::Antialiasing);

    // Линии
    p.setPen(QPen(Qt::black, 2));
    for (auto& l : drawLines)
        for (int i = 1; i < l.points.size(); ++i)
            p.drawLine(l.points[i - 1], l.points[i]);

    // Станции
    for (auto& s : drawStations)
    {
        p.setBrush(Qt::white);
        p.drawEllipse(s.pos, 6, 6);
        p.drawText(s.pos + QPointF(-20, -10), s.name);
    }

    // Поезда
    for (auto& t : drawTrains)
    {
        p.setBrush(t.delayed ? Qt::red : Qt::blue);
        p.drawEllipse(t.pos, 5, 5);
        p.drawText(t.pos + (t.forward ? QPointF(-8, -8) : QPointF(8, 8)), t.id);
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    // Сначала обновляем буфер
    paintSceneBuffer();

    // А затем выводим на экран за один вызов
    QPainter p(this);
    p.drawPixmap(0, 0, sceneBuffer);
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

bool Widget::applySimParams(int start, int step, int duration)
{
    int interval = INT_MAX;

    if (schedule)
    {
        const Entry& e = schedule->getCurrentEntry(start);
        interval = e.interval;
    }

    if (interval > 0 && step >= interval)
    {
        timer.stop();
        QMessageBox *box = new QMessageBox(
            QMessageBox::Critical,
            "Ошибка параметров симуляции",
            "Шаг симуляции должен быть строго меньше интервала движения поездов.\n\n"
            "Интервал: " + QString::number(interval) + " сек\n"
                "Заданный шаг: " + QString::number(step) + " сек\n\n"
                "Работа программы будет завершена.",
            QMessageBox::Ok,
            this
            );

        box->setAttribute(Qt::WA_DeleteOnClose);

        connect(box, &QMessageBox::finished, this, [] {
            QCoreApplication::quit();
        });

        box->open();   // НЕ exec()
        return false;
    }

    simStep = max(1, step);
    simDuration = duration;
    simEndTime = start + duration;

    if (timeController)
        timeController->setCurrent(start);

    return true;
}


void Widget::onStartClicked()
{
    startSimulation();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    const QPointF click = event->pos();

    // 1. Проверка поездов (приоритет)
    for (const auto& t : drawTrains)
    {
        if (QLineF(click, t.pos).length() <= TRAIN_RADIUS + 3)
        {
            QString info =
                "Поезд: " + t.id + "\n" +
                "Направление: " + (t.forward ? "прямое" : "обратное") + "\n" +
                "Статус: " + (t.delayed ? "задержка" : "норма");

            QMessageBox::information(this, "Информация о поезде", info);
            return;
        }
    }

    // 2. Проверка станций
    for (const auto& s : drawStations)
    {
        if (QLineF(click, s.pos).length() <= STATION_RADIUS + 3)
        {
            QString info = "Станция: " + s.name + "\n";

            QMessageBox::information(this, "Информация о станции", info);
            return;
        }
    }
}
