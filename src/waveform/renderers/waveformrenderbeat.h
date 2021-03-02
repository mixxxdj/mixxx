#pragma once

#include <QColor>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformbeat.h"
#include "waveform/renderers/waveformbeatmarker.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBeat();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_beatColor;
    QVector<WaveformBeat> m_beats;
    QVector<WaveformBeatMarker> m_beatMarkers;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
