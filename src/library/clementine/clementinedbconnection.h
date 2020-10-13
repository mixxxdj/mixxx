#pragma once

#include <QSqlDatabase>
#include <QUrl>
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"

class ClementineDbConnection
{
public:

    struct Playlist {
        QString playlistId;
        QString name;
    };

    struct PlaylistEntry {
        int trackId;
        QString title;
        QUrl uri;
        int duration;
        int year;
        int rating;
        QString genre;
        QString grouping;
        int tracknumber;
        int dateadded;
        int bpm;
        int bitrate;
        QString comment;
        int playcount;
        QString composer;
        QString artist;
        QString album;
        QString albumartist;
    };

    ClementineDbConnection();
    virtual ~ClementineDbConnection();
    void setTrackCollection(TrackCollectionManager* pTrackCollection);

    static QString getDatabaseFile();

    bool open(const QString& databaseFile);
    QList<struct Playlist> getPlaylists();
    QList<struct PlaylistEntry> getPlaylistEntries(int playlistId);

private:
    QSqlDatabase m_database;
    TrackCollectionManager* m_pTrackCollectionManager;

};