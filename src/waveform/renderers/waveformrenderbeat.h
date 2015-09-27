#ifndef WAVEFORMRENDERBEAT_H
#define WAVEFORMRENDERBEAT_H

#include <QColor>

#include "waveform/renderers/waveformrendererabstract.h"
#include "util.h"
#include "skin/skincontext.h"

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBeat();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_beatColor;
    QVector<QLineF> m_beats;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};

#endif //WAVEFORMRENDERBEAT_H
