
#ifndef WAVEFORMRENDERSIGNAL_H
#define WAVEFORMRENDERSIGNAL_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QList>
#include <QLineF>

#include "renderobject.h"

class QDomNode;
class QPainter;
class QPaintEvent;


class ControlObjectThreadMain;
class WaveformRenderer;
class SoundSourceProxy;

class WaveformRenderSignal : public RenderObject {
    Q_OBJECT
public:
    WaveformRenderSignal(const char *group, WaveformRenderer *parent);
    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

private:
    WaveformRenderer *m_pParent;
    int m_iWidth, m_iHeight;
    QVector<QLineF> m_lines;
    TrackPointer m_pTrack;
    QColor signalColor;
};

#endif
