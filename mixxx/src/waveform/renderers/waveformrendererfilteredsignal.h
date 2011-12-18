#ifndef WAVEFORMRENDERERFILTEREDSIGNAL_H
#define WAVEFORMRENDERERFILTEREDSIGNAL_H

#include <QColor>
#include <QLineF>

#include <vector>

#include "waveformrendererabstract.h"

class WaveformRendererFilteredSignal : public WaveformRendererAbstract {
  public:
    explicit WaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidget);
    virtual ~WaveformRendererFilteredSignal();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onResize();

  private:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;

    std::vector<QLineF> m_lowLines;
    std::vector<QLineF> m_midLines;
    std::vector<QLineF> m_highLines;

    std::vector<QPointF> m_polygon[3];
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
