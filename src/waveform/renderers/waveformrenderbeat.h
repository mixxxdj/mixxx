#pragma once

#include <QColor>

#include "control/pollingcontrolproxy.h"
#include "skin/legacy/skincontext.h"
#include "track/beats.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderBeat : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderBeat();

    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    int computeAnchorBeatIndex(const mixxx::BeatsPointer& pBeats) const;

    QColor m_beatColor;
    QColor m_downbeatColor;
    PollingControlProxy m_introStartPosCO;
    QVector<QLineF> m_beats;
    QVector<QLineF> m_downbeats;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
