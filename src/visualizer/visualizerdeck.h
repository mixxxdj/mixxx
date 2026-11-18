#pragma once

#include <QImage>
#include <QObject>

#include "control/pollingcontrolproxy.h"
#include "track/track_decl.h"

class BaseTrackPlayer;
class CoverInfo;
QT_FORWARD_DECLARE_CLASS(QPixmap);

namespace mixxx {

/// [AI-generated] What the visualizer window needs to know about one deck:
/// the cover of the loaded track, how loud the bass, the mids and the highs
/// of the deck currently are in the main mix and a phase that advances with
/// the tempo of the deck. The levels are read from the band meters of the
/// engine, see EngineBandMeter. [/AI-generated]
class VisualizerDeck : public QObject {
    Q_OBJECT
  public:
    VisualizerDeck(BaseTrackPlayer* pPlayer, QObject* pParent);
    ~VisualizerDeck() override = default;

    /// [AI-generated] Reads the band meters and the tempo and advances the
    /// smoothing of the levels and the flow phase by the time that has
    /// passed since the previous call. [/AI-generated]
    void update(double elapsedSeconds);

    /// [AI-generated] The smoothed band levels, scaled so that typical music
    /// at full volume peaks around 1. They are 0 while the band is silent.
    /// [/AI-generated]
    float bassLevel() const {
        return m_bassLevel;
    }
    float midLevel() const {
        return m_midLevel;
    }
    float highLevel() const {
        return m_highLevel;
    }

    /// [AI-generated] How loud the deck is in the main mix. This follows the
    /// music slowly and is used to blend the covers of several decks.
    /// [/AI-generated]
    float loudness() const {
        return m_loudness;
    }

    /// [AI-generated] A phase in radians, 0..2 pi, that completes a cycle
    /// every few beats of the deck. It drives the flow of the background.
    /// [/AI-generated]
    float flowPhase() const {
        return m_flowPhase;
    }

    /// [AI-generated] The cover of the loaded track, or a null image if no
    /// track is loaded. [/AI-generated]
    const QImage& cover() const {
        return m_cover;
    }

    /// [AI-generated] A small blurred copy of cover() for the background.
    /// [/AI-generated]
    const QImage& blurredCover() const {
        return m_blurredCover;
    }

    /// [AI-generated] Changes whenever cover() and blurredCover() return
    /// different images. [/AI-generated]
    int coverRevision() const {
        return m_coverRevision;
    }

    /// [AI-generated] Returns how strongly a band distorts the visuals, 0..1.
    /// The intensity is the maximum the user allows for the band and the
    /// level is how loud the band actually is. The result is 0 if either of
    /// them is 0. [/AI-generated]
    static float bandAmount(double intensity, float level);

    /// [AI-generated] Returns the brightness of the cover of a deck, 0..1,
    /// from the loudness of the deck and the summed loudness of all decks.
    /// The weights of all decks sum up to 1 at most and fade to 0 when the
    /// main mix becomes silent. [/AI-generated]
    static float mixWeight(float loudness, float totalLoudness);

    /// [AI-generated] Returns the flow phase after the given time at the
    /// given tempo, wrapped to 0..2 pi. A tempo of 0 means that the tempo is
    /// unknown; the phase then advances at a moderate default tempo.
    /// [/AI-generated]
    static float advancedFlowPhase(float flowPhase, double bpm, double elapsedSeconds);

  private slots:
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackCoverArtUpdated();
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  private:
    void setCover(const QImage& cover);

    PollingControlProxy m_bassMeter;
    PollingControlProxy m_midMeter;
    PollingControlProxy m_highMeter;
    PollingControlProxy m_bpm;

    float m_bassLevel;
    float m_midLevel;
    float m_highLevel;
    float m_loudness;
    float m_flowPhase;

    TrackPointer m_pLoadedTrack;
    QImage m_cover;
    QImage m_blurredCover;
    int m_coverRevision;
};

} // namespace mixxx
