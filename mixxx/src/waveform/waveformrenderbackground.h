
#ifndef WAVEFORMRENDERBACKGROUND_H
#define WAVEFORMRENDERBACKGROUND_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QPixmap>

#include "renderobject.h"

class QDomNode;
class QPainter;
class QPaintEvent;

class WaveformRenderer;

class WaveformRenderBackground : public RenderObject {
    Q_OBJECT
  public:
    WaveformRenderBackground(const char *group, WaveformRenderer *parent);
    virtual ~WaveformRenderBackground();

    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

  private:
    void generatePixmap();
    int m_iWidth, m_iHeight;
    QColor bgColor;
    QPixmap m_backgroundPixmap;
    QString m_backgroundPixmapPath;
    bool m_bRepaintBackground;
};

#endif
