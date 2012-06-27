
#ifndef WAVEFORMRENDERSIGNALPIXMAP_H
#define WAVEFORMRENDERSIGNALPIXMAP_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QList>
#include <QLineF>
#include <QPixmap>

#include "renderobject.h"

class QDomNode;
class QPainter;
class QPaintEvent;


class ControlObjectThreadMain;
class WaveformRenderer;
class SoundSourceProxy;

class WaveformRenderSignalPixmap : public RenderObject {
    Q_OBJECT
  public:
    WaveformRenderSignalPixmap(const char *group, WaveformRenderer *parent);
    virtual ~WaveformRenderSignalPixmap();

    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

  private:
    void updatePixmap(QVector<float> *buffer, double playPos, double rateAdjust);

    WaveformRenderer *m_pParent;
    int m_iWidth, m_iHeight;
    int m_iLastPlaypos;
    QVector<QLineF> m_lines;
    TrackPointer m_pTrack;
    QColor signalColor;
    QPixmap m_screenPixmap;
};

#endif
