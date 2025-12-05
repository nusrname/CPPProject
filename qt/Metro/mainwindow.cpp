#include "MainWindow.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    scene(new QGraphicsScene(this)),
    timer(new QTimer(this))
{
    // создаём графический виджет
    auto *view = new QGraphicsView(scene, this);
    setCentralWidget(view);

    setupScene();
    setupTimer();
}

MainWindow::~MainWindow() {}

void MainWindow::setupScene()
{
    scene->setSceneRect(0, 0, 1200, 400);

    // Пример линии метро
    scene->addLine(50, 200, 1150, 200, QPen(Qt::black, 4));

    // Пример станции
    scene->addEllipse(100, 180, 40, 40, QPen(Qt::blue), QBrush(Qt::white));
    scene->addEllipse(300, 180, 40, 40, QPen(Qt::blue), QBrush(Qt::white));
    scene->addEllipse(500, 180, 40, 40, QPen(Qt::blue), QBrush(Qt::white));

    // Пример поезда
    auto *train = scene->addRect(70, 190, 20, 20, QPen(Qt::red), QBrush(Qt::red));
    train->setData(0, "train1"); // можно хранить id
}

void MainWindow::setupTimer()
{
    connect(timer, &QTimer::timeout, this, &MainWindow::onUpdate);
    timer->start(100); // обновление каждые 100 мс
}

void MainWindow::onUpdate()
{
    // пример движения поезда вправо
    for (auto *item : scene->items())
    {
        if (item->data(0) == "train1")
        {
            item->moveBy(1, 0);
        }
    }
}
