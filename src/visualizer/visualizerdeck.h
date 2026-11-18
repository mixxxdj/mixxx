#pragma once

#include <QFutureWatcher>
#include <QImage>
#include <QObject>

#include "control/pollingcontrolproxy.h"
#include "track/track_decl.h"

class BaseTrackPlayer;
class CoverInfo;
QT_FORWARD_DECLARE_CLASS(QPixmap);

namespace mixxx {

/// [AI-generated] What the visualizer window needs to know about one deck:
/// the cover of the loaded track, the tempo of the deck and how loud its
/// bass, mids and highs currently are in the main mix. The levels are read
/// from the band meters of the engine, see EngineBandMeter. The cover is
/// only loaded while the deck is active, and it is prepared for drawing in a
/// worker thread. [/AI-generated]
class VisualizerDeck : public QObject {
    Q_OBJECT
  public:
    VisualizerDeck(BaseTrackPlayer* pPlayer, QObject* pParent);
    ~VisualizerDeck() override = default;

    /// [AI-generated] Tells the deck whether its cover is needed, which is
    /// the case while the visualizer window is shown. An inactive deck
    /// keeps track of the loaded track, but does not load covers.
    /// [/AI-generated]
    void setActive(bool active);

    /// [AI-generated] Reads the band meters and advances the smoothing of
    /// the levels by the time that has passed since the previous call.
    /// [/AI-generated]
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
    /// music slowly and decides how much of the cover of the deck is shown.
    /// [/AI-generated]
    float loudness() const {
        return m_loudness;
    }

    /// [AI-generated] The current tempo of the deck in beats per minute, or
    /// 0 if it is unknown. [/AI-generated]
    double bpm() const {
        return m_bpm.get();
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

    /// [AI-generated] Returns the phase that drives the flow of the
    /// background after the given time at the given tempo. The phase is in
    /// radians, wrapped to 0..2 pi, and completes a cycle every few beats. A
    /// tempo of 0 means that the tempo is unknown; the phase then advances
    /// at a moderate default tempo. [/AI-generated]
    static float advancedFlowPhase(float flowPhase, double bpm, double elapsedSeconds);

  private slots:
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackCoverArtUpdated();
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
    void slotCoverPrepared();

  private:
    /// [AI-generated] A cover that is ready for drawing. The request number
    /// tells whether it is still the cover that the deck is waiting for.
    /// [/AI-generated]
    struct PreparedCover {
        int m_request;
        QImage m_cover;
        QImage m_blurredCover;
    };

    static PreparedCover prepareCover(int request, QImage cover);

    void requestCover();
    void clearCover();

    PollingControlProxy m_bassMeter;
    PollingControlProxy m_midMeter;
    PollingControlProxy m_highMeter;
    PollingControlProxy m_bpm;

    float m_bassLevel;
    float m_midLevel;
    float m_highLevel;
    float m_loudness;

    TrackPointer m_pLoadedTrack;
    QImage m_cover;
    QImage m_blurredCover;
    int m_coverRevision;

    bool m_active;
    bool m_coverRequested;
    int m_coverRequest;
    QFutureWatcher<PreparedCover> m_preparedCoverWatcher;
};

} // namespace mixxx
