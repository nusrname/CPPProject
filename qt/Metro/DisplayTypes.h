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
    bool forward;
};

struct DrawLine
{
    QVector<QPointF> points;
};
