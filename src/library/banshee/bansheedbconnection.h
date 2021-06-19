#pragma once

#include <QSqlDatabase>
#include <QUrl>

class BansheeDbConnection
{
public:

    struct Playlist {
        QString playlistId;
        QString name;
    };

    struct Track {
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
    };

    struct Artist {
        QString name;
    };

    struct Album {
        QString title;
    };

    struct PlaylistEntry {
        int trackId;
        int viewOrder;
        struct Track* pTrack;
        struct Artist* pArtist;
        struct Album* pAlbum;
        struct Artist* pAlbumArtist;
    };

    BansheeDbConnection();
    virtual ~BansheeDbConnection();

    static QString getDatabaseFile();

    bool open(const QString& databaseFile);
    int getSchemaVersion();
    QList<Playlist> getPlaylists();
    QList<PlaylistEntry> getPlaylistEntries(int playlistId);

private:
    QSqlDatabase m_database;
    QMap<int, Track> m_trackMap;
    QMap<int, Artist> m_artistMap;
    QMap<int, Album> m_albumMap;

};
