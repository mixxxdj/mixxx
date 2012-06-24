#ifndef WAVEFORMRENDERERFILTEREDSIGNAL_H
#define WAVEFORMRENDERERFILTEREDSIGNAL_H

#include "waveformrenderersignalbase.h"

#include <vector>
#include <QLineF>

#include "util.h"

class WaveformRendererFilteredSignal : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidget);
    virtual ~WaveformRendererFilteredSignal();

    virtual void onInit();
    virtual void onSetup(const QDomNode& node);

    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onResize();

  private:
    std::vector<QLineF> m_lowLines;
    std::vector<QLineF> m_midLines;
    std::vector<QLineF> m_highLines;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererFilteredSignal);
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
