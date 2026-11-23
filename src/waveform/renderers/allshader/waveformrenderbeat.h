#pragma once

#include <QColor>

#include "audio/frame.h"
#include "control/pollingcontrolproxy.h"
#include "rendergraph/geometrynode.h"
#include "track/beats.h"
#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

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
    void slotBeatsUpdated();
    void slotCuesUpdated();

  private:
    void setFirstDownbeatMaybeInvalid();

    QColor m_color;
    QColor m_downbeatColor;
    bool m_isSlipRenderer;
    mixxx::BeatsPointer m_pTrackBeats;
    // members for setting/caching the downbeat at intro_start_position
    PollingControlProxy m_introStartPosCO;
    std::optional<mixxx::Beats::ConstIterator> m_firstDownBeat;

    TrackPointer m_pLoadedTrack;

    // Optional skin styling of beat and bar (downbeat) lines
    enum class MarkerShape {
        None,
        Triangle,
        Tick,
        Block,
    };
    struct LineStyle {
        float lineWidth{1.f}; // 0 draws the markers only
        MarkerShape markerShape{MarkerShape::None};
        float markerSize{4.f};
        QColor markerColor;
    };
    LineStyle m_beatStyle;
    LineStyle m_barStyle;
    bool m_markersOnTop{true};
    bool m_markersOnBottom{true};

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
