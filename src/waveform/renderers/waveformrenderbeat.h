#ifndef WAVEFORMRENDERBEAT_H
#define WAVEFORMRENDERBEAT_H

#include <QColor>

#include "waveform/renderers/waveformrendererabstract.h"
#include "util.h"
#include "skin/skincontext.h"

class ControlObjectThread;

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBeat();

    virtual bool init();
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_beatColor;
    ControlObjectThread* m_pBeatActive;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};

#endif //WAVEFORMRENDERBEAT_H
