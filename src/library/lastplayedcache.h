#pragma once

#include <QObject>
#include <QSet>
#include <QSqlDatabase>
#include <QString>

#include "track/track.h"
#include "track/trackid.h"

class TrackCollection;

const QString LASTPLAYEDTABLE_NAME = "last_played";

/// LastPlayedCache is a helper object for creating and maintaining
/// the "last_played" temporary view on the database. It creates the
/// view and also watches for changes to the Setlog Playlists and updates
/// tracks in the cache as necessary.
class LastPlayedCache : public QObject {
    Q_OBJECT
  public:
    LastPlayedCache(TrackCollection* trackCollection);
    ~LastPlayedCache() {
    }

    static QDateTime fetchLastPlayedTime(const QSqlDatabase& db, TrackPointer pTrack);

  public slots:
    void slotPlaylistTrackChanged(int playlistId, TrackId trackId, int position);

  private:
    void initTableView();

    TrackCollection* const m_pTrackCollection;
};
