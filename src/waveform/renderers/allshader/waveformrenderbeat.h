#pragma once

#include <QColor>

#include "control/pollingcontrolproxy.h"
#include "rendergraph/geometrynode.h"
#include "track/beats.h"
#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

namespace rendergraph {
class GeometryNode;
} // namespace rendergraph

namespace allshader {
class WaveformRenderBeat;
} // namespace allshader

class allshader::WaveformRenderBeat final
        : public QObject,
          public ::WaveformRendererAbstract,
          public rendergraph::GeometryNode {
    Q_OBJECT
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    void onSetTrack() override;

    // Virtuals for rendergraph::Node
    void preprocess() override;

  public slots:
    void setColor(const QColor& color) {
        m_color = color;
    }
    void setDownbeatColor(const QColor& color) {
        m_downbeatColor = color;
    }
    void setBeatsPerBar(int beatsPerBar) {
        m_beatsPerBar = beatsPerBar;
    }
    void setDownbeatsEnabled(bool enabled) {
        m_downbeatsEnabled = enabled;
    }
    void slotBeatsUpdated();
    void slotCuesUpdated();

  private:
    void updateDownbeatAnchor();

    QColor m_color;
    QColor m_downbeatColor;
    int m_beatsPerBar{4};
    bool m_downbeatsEnabled{true};
    bool m_isSlipRenderer;

    rendergraph::GeometryNode* m_pDownbeatNode{};

    TrackPointer m_pLoadedTrack;
    mixxx::BeatsPointer m_pTrackBeats;
    PollingControlProxy m_introStartPosCO;
    int m_anchorBeatIndex{0};

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
