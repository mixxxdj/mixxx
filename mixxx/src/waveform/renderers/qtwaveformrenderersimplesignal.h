#ifndef QTWAVEFORMRENDERERSIMPLESIGNAL_H
#define QTWAVEFORMRENDERERSIMPLESIGNAL_H

#include "waveformrenderersignalbase.h"

#include <QBrush>
#include <QPen>

#include <vector>

class ControlObject;

class QtWaveformRendererSimpleSignal : public WaveformRendererSignalBase {
public:
    explicit QtWaveformRendererSimpleSignal( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~QtWaveformRendererSimpleSignal();

    virtual void onInit();
    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

protected:
    virtual void onResize();

private:
    QBrush m_brush;
    QPen m_borderPen;
    std::vector<QPointF> m_polygon;
};

#endif // QTWAVEFORMRENDERERSIMPLESIGNAL_H
