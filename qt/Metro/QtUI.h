#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <vector>
#include <memory>

class Metro;

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

    void setMetro(std::shared_ptr<Metro> m);   // <<---- НОВОЕ

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::Widget *ui;

    std::shared_ptr<Metro> metro;

    // Данные для отрисовки
    std::vector<DrawLine> drawLines;
    std::vector<DrawStation> drawStations;
    std::vector<DrawTrain> drawTrains;

    void rebuildScene();    // <<---- НОВОЕ
};

#endif // WIDGET_H
