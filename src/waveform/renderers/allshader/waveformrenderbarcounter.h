#pragma once

#include <QColor>
#include <QObject>

#include "control/pollingcontrolproxy.h"
#include "rendergraph/node.h"
#include "track/beats.h"
#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRenderBarCounter;
} // namespace allshader

class allshader::WaveformRenderBarCounter final
        : public QObject,
          public ::WaveformRendererAbstract,
          public rendergraph::Node {
    Q_OBJECT
  public:
    explicit WaveformRenderBarCounter(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    void onSetTrack() override;

    // Virtual for rendergraph::Node
    void preprocess() override;

  public slots:
    void setColor(const QColor& color) {
        m_color = color;
    }
    void setBeatsPerBar(int beatsPerBar) {
        m_beatsPerBar = beatsPerBar;
    }
    void setDownbeatsEnabled(bool enabled) {
        m_downbeatsEnabled = enabled;
    }
    void setShowBarCounter(bool show) {
        m_showBarCounter = show;
    }
    void slotBeatsUpdated();
    void slotCuesUpdated();

  private:
    bool preprocessInner();
    void removeAllChildNodes();
    void updateDownbeatAnchor();

    QColor m_color;
    // Global fallback time signature; set from WaveformWidgetFactory in the
    // constructor and kept in sync via beatsPerBarChanged. 0 until then.
    int m_beatsPerBar{0};
    bool m_downbeatsEnabled{true};
    bool m_showBarCounter{true};
    bool m_isSlipRenderer;

    TrackPointer m_pLoadedTrack;
    mixxx::BeatsPointer m_pTrackBeats;
    PollingControlProxy m_introStartPosCO;
    int m_anchorBeatIndex{0};

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBarCounter);
};
