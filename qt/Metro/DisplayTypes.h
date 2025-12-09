#pragma once
#include <QString>
#include <QPointF>

struct DrawStation {
    QString name;
    QPointF pos;
};

struct DrawTrain {
    QString id;
    QPointF pos;
    bool forward;
};

struct DrawLine {
    QVector<QPointF> points;
};
