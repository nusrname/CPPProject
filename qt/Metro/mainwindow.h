#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QGraphicsScene *scene;
    QTimer *timer;

    void setupScene();
    void setupTimer();

private slots:
    void onUpdate();
};
