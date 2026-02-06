#pragma once

#include <QList>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include "preferences/usersettings.h"
#include "track/trackid.h"

class TrackCollectionManager;

namespace mixxx {

/// Worker thread for syncing ratings from file tags to database.
/// Runs asynchronously to avoid blocking the UI when opening playlists/crates.
/// File tags are treated as the source of truth - ratings are always imported
/// from files when the preference is enabled.
class RatingSyncWorker : public QThread {
    Q_OBJECT

  public:
    RatingSyncWorker(
            TrackCollectionManager* pTrackCollectionManager,
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);
    ~RatingSyncWorker() override;

    /// Queue track IDs for rating sync.
    /// If called while a sync is in progress, the new tracks replace
    /// the pending queue (expected UX when user clicks a different playlist).
    void syncRatingsForTracks(const QList<TrackId>& trackIds);

    /// Stop the worker thread gracefully.
    void stop();

  signals:
    /// Emitted when sync is complete for a batch.
    /// tracksUpdated: number of tracks whose ratings were updated
    /// totalTracks: total number of tracks processed
    void syncComplete(int tracksUpdated, int totalTracks);

  protected:
    void run() override;

  private:
    TrackCollectionManager* m_pTrackCollectionManager;
    UserSettingsPointer m_pConfig;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QList<TrackId> m_pendingTrackIds;
    bool m_stop;
};

} // namespace mixxx
