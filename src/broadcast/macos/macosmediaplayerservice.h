#pragma once

#include <QList>
#include <QObject>

#include "control/controlproxy.h"
#include "library/autodj/autodjprocessor.h"
#include "library/coverart.h"
#include "mixer/playermanager.h"
#include "util/cache.h"

// TODO: Implement ScrobblingService once merged

/// A service that interfaces with macOS's media controls and allows the user to
/// view and control the playback through the system-wide control center, media
/// keys etc.
class MacOSMediaPlayerService : public QObject {
    Q_OBJECT
  public:
    MacOSMediaPlayerService(PlayerManagerInterface& pPlayerManager);
    ~MacOSMediaPlayerService() override;

  public slots:
    /// Sends the updated track info to the external 'now playing' center.
    void slotBroadcastCurrentTrack(TrackPointer pTrack); // TODO: override
    /// Notifies the external 'now playing' center that all tracks are paused.
    void slotAllTracksPaused(); // TODO: override

  private:
    QList<DeckAttributes*> m_deckAttributes;
    ControlProxy* m_pCPAutoDjEnabled;
    ControlProxy* m_pCPFadeNow;
    double m_lastSentPosition;

    /// Sets up the callbacks for controlling playback externally.
    void setupCommandHandlers();

    /// Fetches the currently playing deck. Returns a `nullptr` if nothing plays.
    DeckAttributes* getCurrentDeck();
    /// Checks whether the given deck is the currently playing one.
    bool isCurrentDeck(DeckAttributes* attributes);

    /// Called when the play state is toggled externally. Returns whether this was successful.
    bool togglePlayState();
    /// Called when the play state is changed externally. Returns whether this was successful.
    bool updatePlayState(bool playing);
    /// Called when the play position is changed externally. Returns whether this was successful.
    bool updatePlayPosition(double absolutePosition);
    /// Called when the user skips to the next track externally. Returns whether
    /// this was successful.
    bool skipToNextTrack();

  private slots:
    /// Notifies the external 'now playing' center that the play state of some deck has changed.
    void slotPlayChanged(DeckAttributes* attributes, bool playing);
    /// Notifies the external 'now playing' center that the play position of some deck has changed.
    void slotPlayPositionChanged(DeckAttributes* attributes, double position);

    /// Updates the cover art if needed.
    void slotCoverFound(const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
};
