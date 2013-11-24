#ifndef BANSHEEDBCONNECTION_H
#define BANSHEEDBCONNECTION_H

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
    QList<struct Playlist> getPlaylists();
    QList<struct PlaylistEntry> getPlaylistEntries(int playlistId);

    static bool viewOrderLessThen(struct PlaylistEntry & s1, struct PlaylistEntry& s2) {
        return s1.viewOrder < s2.viewOrder;
    }
    static bool viewOrderGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.viewOrder > s2.viewOrder;
    }

    static bool artistLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pArtist->name < s2.pArtist->name;
    }
    static bool artistGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pArtist->name > s2.pArtist->name;
    }

    static bool albumArtistLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pAlbumArtist->name < s2.pAlbumArtist->name;
    }
    static bool albumArtistGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pAlbumArtist->name > s2.pAlbumArtist->name;
    }

    static bool titleLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->title < s2.pTrack->title;
    }
    static bool titleGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->title > s2.pTrack->title;
    }

    static bool durationLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->duration < s2.pTrack->duration;
    }
    static bool durationGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->duration > s2.pTrack->duration;
    }

    static bool uriLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->uri.toString() < s2.pTrack->uri.toString();
    }
    static bool uriGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->uri.toString() > s2.pTrack->uri.toString();
    }

    static bool albumLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pAlbum->title < s2.pAlbum->title;
    }
    static bool albumGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pAlbum->title > s2.pAlbum->title;
    }

    static bool yearLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->year < s2.pTrack->year;
    }
    static bool yearGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->year > s2.pTrack->year;
    }

    static bool ratingLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->rating < s2.pTrack->rating;
    }
    static bool ratingGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->rating > s2.pTrack->rating;
    }

    static bool genreLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->genre < s2.pTrack->genre;
    }
    static bool genreGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->genre > s2.pTrack->genre;
    }

    static bool groupingLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->grouping < s2.pTrack->grouping;
    }
    static bool groupingGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->grouping > s2.pTrack->grouping;
    }

    static bool tracknumberLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->tracknumber < s2.pTrack->tracknumber;
    }
    static bool tracknumberGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->tracknumber > s2.pTrack->tracknumber;
    }

    static bool dateaddedLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->dateadded < s2.pTrack->dateadded;
    }
    static bool dateaddedGreaterThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->dateadded > s2.pTrack->dateadded;
    }

    static bool bpmLessThen(struct PlaylistEntry& s1, struct PlaylistEntry& s2) {
        return s1.pTrack->bpm < s2.pTrack->bpm;
    }
    ;
    static bool bpmGreaterThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->bpm > s2.pTrack->bpm;
    }

    static bool bitrateLessThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->bitrate < s2.pTrack->bitrate;
    }
    static bool bitrateGreaterThen(struct PlaylistEntry& s1,PlaylistEntry& s2) {
        return s1.pTrack->bitrate > s2.pTrack->bitrate;
    }

    static bool commentLessThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->comment < s2.pTrack->comment;
    }
    static bool commentGreaterThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->comment > s2.pTrack->comment;
    }

    static bool playcountLessThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->playcount < s2.pTrack->playcount;
    }
    static bool playcountGreaterThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->playcount > s2.pTrack->playcount;
    }

    static bool composerLessThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->composer < s2.pTrack->composer;
    }
    static bool composerGreaterThen(struct PlaylistEntry& s1, PlaylistEntry& s2) {
        return s1.pTrack->composer > s2.pTrack->composer;
    }

private:
    QSqlDatabase m_database;
    QMap<int, struct Track> m_trackMap;
    QMap<int, struct Artist> m_artistMap;
    QMap<int, struct Album> m_albumMap;

};

#endif // BANSHEEDBCONNECTION_H
