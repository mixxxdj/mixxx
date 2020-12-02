#ifndef QTWAVEFROMRENDERERFILTEREDSIGNAL_H
#define QTWAVEFROMRENDERERFILTEREDSIGNAL_H

#include <QBrush>
#include <QDomNode>
#include <QPointF>
#include <QVector>
#include <QtGui>

#include "waveformrenderersignalbase.h"

class ControlObject;
class WaveformWidgetRenderer;

class QtWaveformRendererFilteredSignal : public WaveformRendererSignalBase {
  public:
    explicit QtWaveformRendererFilteredSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~QtWaveformRendererFilteredSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  protected:
    virtual void onResize();
    int buildPolygon();

  protected:
    QBrush m_lowBrush;
    QBrush m_midBrush;
    QBrush m_highBrush;
    QBrush m_lowKilledBrush;
    QBrush m_midKilledBrush;
    QBrush m_highKilledBrush;

    QVector<QPointF> m_polygon[3];
};

#endif // QTWAVEFROMRENDERERFILTEREDSIGNAL_H
