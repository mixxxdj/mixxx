#ifndef QTWAVEFORMRENDERERSIMPLESIGNAL_H
#define QTWAVEFORMRENDERERSIMPLESIGNAL_H

#include <QBrush>
#include <QDomNode>
#include <QPen>
#include <QPointF>
#include <QVector>
#include <QtGui>

#include "waveformrenderersignalbase.h"

class ControlObject;
class WaveformWidgetRenderer;

class QtWaveformRendererSimpleSignal : public WaveformRendererSignalBase {
public:
    explicit QtWaveformRendererSimpleSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~QtWaveformRendererSimpleSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

protected:
    virtual void onResize();

private:
    QBrush m_brush;
    QPen m_borderPen;
    QVector<QPointF> m_polygon;
};

#endif // QTWAVEFORMRENDERERSIMPLESIGNAL_H
