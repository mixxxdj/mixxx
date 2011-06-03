#ifndef WAVEFORMRENDERBACKGROUND_H
#define WAVEFORMRENDERBACKGROUND_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QPixmap>

//#include "renderobject.h"
#include "waveformrendererabstract.h"

class QDomNode;
class QPainter;
class QPaintEvent;

class WaveformRenderer;

class WaveformRenderBackground : public WaveformRendererAbstract {

public:
    WaveformRenderBackground( WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

private:
    void generatePixmap();

    QColor m_backgroungColor;
    QPixmap m_backgroundPixmap;
};

#endif
