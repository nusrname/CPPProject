#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <vector>
#include <memory>
using namespace std;

class Metro;
class Schedule;
class TrainManager;
class TimeController;

#include "DisplayTypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    void setMetro(shared_ptr<Metro> m,
                  shared_ptr<Schedule> sch,
                  shared_ptr<TrainManager> manager,
                  shared_ptr<TimeController> time);

    void applySimParams(int start, int step, int duration);
    void startSimulation();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onStartClicked();
    void onPauseClicked();
    void tick();

private:
    QLabel *labelParams;
    QLabel *labelTime;
    QPushButton *btnPause;
    QSpinBox *spinStep;

    bool paused = false;
    // simulation
    QTimer timer;
    int simStep = 1;
    int simDuration = 0;
    int simEndTime = 0;

    // data
    shared_ptr<Metro> metro;
    shared_ptr<Schedule> schedule;
    shared_ptr<TrainManager> manager;
    shared_ptr<TimeController> timeController;

    vector<DrawLine> drawLines;
    vector<DrawStation> drawStations;
    vector<DrawTrain> drawTrains;

    void rebuildScene();
    void updateTrainsOnScene();
    static constexpr double PIXELS_PER_SECOND = 0.5;
};

class StartDialog : public QDialog
{
    Q_OBJECT
public:
    QLineEdit *editStart, *editStep, *editDuration;
    StartDialog(QWidget *parent = nullptr);
};

#endif // WIDGET_H
