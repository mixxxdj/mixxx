#pragma once

#include "waveformrenderersignalbase.h"

#include <QBrush>
#include <QPen>

#include <QVector>

class ControlObject;

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
