#pragma once

#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <chrono>

#include "control/pollingcontrolproxy.h"
#include "util/parented_ptr.h"

class BaseTrackPlayer;
QT_FORWARD_DECLARE_CLASS(QPixmap);

namespace mixxx {

class PlayerCoverArtWatcher;

/// What the visualizer window needs to know about one deck: the cover of the
/// loaded track, the tempo of the deck and how loud its bass, mids and highs
/// currently are in the main mix. The levels are read from the band meters of
/// the engine, see EngineBandMeter. The cover is only loaded while the deck is
/// active, and it is prepared for drawing in a worker thread.
class VisualizerDeck : public QObject {
    Q_OBJECT
  public:
    VisualizerDeck(BaseTrackPlayer* pPlayer, QObject* pParent);
    ~VisualizerDeck() override = default;

    /// Tells the deck whether its cover is needed, which is the case while the
    /// visualizer window is shown. An inactive deck keeps track of the loaded
    /// track, but does not load covers.
    void setActive(bool active);

    /// Reads the band meters and advances the smoothing of the levels by the
    /// time that has passed since the previous call.
    void update(std::chrono::duration<double> elapsed);

    /// The smoothed band levels, scaled so that typical music at full volume
    /// peaks around 1. They are 0 while the band is silent.
    float bassLevel() const {
        return m_bassLevel;
    }
    float midLevel() const {
        return m_midLevel;
    }
    float highLevel() const {
        return m_highLevel;
    }

    /// How loud the deck is in the main mix. This follows the music slowly and
    /// decides how much of the cover of the deck is shown.
    float loudness() const {
        return m_loudness;
    }

    /// The current tempo of the deck in beats per minute, or 0 if it is
    /// unknown.
    double bpm() const {
        return m_bpm.get();
    }

    /// The cover of the loaded track, or a null image if no track is loaded.
    const QImage& cover() const {
        return m_cover;
    }

    /// A small blurred copy of cover() for the background.
    const QImage& blurredCover() const {
        return m_blurredCover;
    }

    /// Changes whenever cover() and blurredCover() return different images.
    int coverRevision() const {
        return m_coverRevision;
    }

    /// Returns how strongly a band distorts the visuals, 0..1. The intensity is
    /// the maximum the user allows for the band and the level is how loud the
    /// band actually is. The result is 0 if either of them is 0.
    static float bandAmount(double intensity, float level);

    /// Returns the phase that drives the flow of the background after the given
    /// time at the given tempo. The phase is in radians, wrapped to 0..2 pi,
    /// and completes a cycle every few beats. A tempo of 0 means that the tempo
    /// is unknown; the phase then advances at a moderate default tempo.
    static float advancedFlowPhase(
            float flowPhase, double bpm, std::chrono::duration<double> elapsed);

    /// A picture that is ready for drawing. The request number tells whether it
    /// is still the picture that its requester is waiting for.
    struct PreparedCover {
        int m_request;
        QImage m_cover;
        QImage m_blurredCover;
    };

    /// Scales a picture down to the size that is used for drawing and creates
    /// its blurred copy. A null picture is replaced by the default cover. This
    /// takes a while for large pictures and is meant to be run in a worker
    /// thread.
    static PreparedCover prepareCover(int request, QImage cover);

  private slots:
    void slotTrackChanged();
    void slotCoverOutdated();
    void slotCoverFound(const QPixmap& pixmap);
    void slotCoverPrepared();

  private:
    void requestCover();

    PollingControlProxy m_bassMeter;
    PollingControlProxy m_midMeter;
    PollingControlProxy m_highMeter;
    PollingControlProxy m_bpm;

    float m_bassLevel;
    float m_midLevel;
    float m_highLevel;
    float m_loudness;

    parented_ptr<PlayerCoverArtWatcher> m_pCoverArtWatcher;
    QImage m_cover;
    QImage m_blurredCover;
    int m_coverRevision;

    bool m_active;
    bool m_coverRequested;
    int m_coverRequest;
    QFutureWatcher<PreparedCover> m_preparedCoverWatcher;
};

} // namespace mixxx
