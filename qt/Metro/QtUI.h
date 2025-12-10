#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <vector>
#include <memory>
using namespace std;

class Metro;
class Schedule;

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

    void setMetro(shared_ptr<Metro> m, shared_ptr<Schedule> sch);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::Widget *ui;

    shared_ptr<Metro> metro;
    shared_ptr<Schedule> schedule;

    vector<DrawLine> drawLines;
    vector<DrawStation> drawStations;
    vector<DrawTrain> drawTrains;

    void rebuildScene();
    static constexpr double PIXELS_PER_SECOND = 2.0;
};

#endif // WIDGET_H
