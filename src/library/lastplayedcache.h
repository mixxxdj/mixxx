#pragma once

#include <QObject>
#include <QSet>
#include <QSqlDatabase>
#include <QString>

#include "library/queryutil.h"
#include "track/track.h"
#include "track/trackid.h"

class TrackCollection;

extern const QString LASTPLAYEDTABLE_NAME;

/// LastPlayedFetcher is a small object for fetching last played times from the
/// database.  It holds and reuses a QSqlQuery for efficiency.
class LastPlayedFetcher {
  public:
    explicit LastPlayedFetcher(QSqlDatabase db)
            : m_fetchQuery(db) {
    }

    /// Fetches the last played time for the given track. Returns an empty (null)
    /// QDateTime on error.
    QDateTime fetch(TrackPointer pTrack);

  private:
    QSqlQuery m_fetchQuery;
};

/// LastPlayedCache is a helper object for creating and maintaining
/// the "last_played" temporary view on the database. It creates the
/// view and also watches for changes to the Setlog Playlists and updates
/// tracks in the cache as necessary.
class LastPlayedCache : public QObject {
    Q_OBJECT
  public:
    explicit LastPlayedCache(TrackCollection* trackCollection);
    ~LastPlayedCache() override = default;

  private:
    void initTableView();
    void playlistTrackChanged(int playlistId, TrackId trackId);

    TrackCollection* const m_pTrackCollection;
    LastPlayedFetcher m_helper;
};
