#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <QObject>
#include <QDomNode>
#include <QVector>

#include "trackinfoobject.h"

class QDomNode;
class QPainter;
class QPaintEvent;

class RenderObject : public QObject {
    Q_OBJECT
    public:
    explicit RenderObject();
    virtual ~RenderObject();
    virtual void resize(int w, int h) = 0;
    virtual void setup(QDomNode node) = 0;
    virtual void draw(QPainter *pPainter, QPaintEvent *pEvent,
                      QVector<float> *buffer, double playpos,
                      double rateAdjust) = 0;
    virtual void newTrack(TrackPointer pTrack) = 0;
};

#endif
