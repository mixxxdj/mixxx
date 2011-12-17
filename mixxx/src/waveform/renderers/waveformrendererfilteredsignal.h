#ifndef WAVEFORMRENDERERFILTEREDSIGNAL_H
#define WAVEFORMRENDERERFILTEREDSIGNAL_H

#include <QColor>
#include <QVector>
#include <QLineF>

#include "util.h"
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

    QVector<QLineF> m_lowLines;
    QVector<QLineF> m_midLines;
    QVector<QLineF> m_highLines;
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
