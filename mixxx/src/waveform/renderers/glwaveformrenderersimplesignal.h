#ifndef GLWAVEFORMRENDERERSIMPLESIGNAL_H
#define GLWAVEFORMRENDERERSIMPLESIGNAL_H

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"

#include <QBrush>
#include <QPen>

#include <vector>

class ControlObject;

class GLWaveformRendererSimpleSignal : public WaveformRendererAbstract {
public:
    explicit GLWaveformRendererSimpleSignal( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererSimpleSignal();

    virtual void init();
    virtual void setup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

protected:
    virtual void onResize();

private:
    WaveformSignalColors m_colors;
    Qt::Alignment m_alignment;

    QBrush m_brush;
    QPen m_borderPen;
    std::vector<QPointF> m_polygon;
};

#endif // GLWAVEFORMRENDERERSIMPLESIGNAL_H
