#pragma once
#include <QString>
#include <QPointF>
#include "../../vs/Line.h"

struct DrawStation
{
    QString name;
    QPointF pos;
    shared_ptr<Station> station;
};


struct DrawTrain
{
    QString id;
    QPointF pos;
    QPointF nextPos;
    bool forward;
    bool aboveLine;
    bool delayed;
};

struct DrawLine
{
    QVector<QPointF> points;
};
