#include "library/itunes/itunesdao.h"

#include <QObject>
#include <QSqlQuery>
#include <gsl/pointers>

#include "library/itunes/ituneslocalhosttoken.h"
#include "library/itunes/itunespathmapping.h"
#include "library/queryutil.h"
#include "library/treeitem.h"

std::ostream& operator<<(std::ostream& os, const ITunesTrack& track) {
    os << "ITunesTrack { "
       << ".id = " << track.id << ", "
       << ".artist = \"" << track.artist.toStdString() << "\", "
       << ".title = \"" << track.title.toStdString() << "\", "
       << ".album = \"" << track.album.toStdString() << "\", "
       << ".albumArtist = \"" << track.albumArtist.toStdString() << "\", "
       << ".composer = \"" << track.composer.toStdString() << "\", "
       << ".genre = \"" << track.genre.toStdString() << "\", "
       << ".grouping = \"" << track.grouping.toStdString() << "\", "
       << ".year = " << track.year << ", "
       << ".duration = " << track.duration << ", "
       << ".location = \"" << track.location.toStdString() << "\", "
       << ".rating = " << track.rating << ", "
       << ".comment = \"" << track.comment.toStdString() << "\", "
       << ".trackNumber = " << track.trackNumber << ", "
       << ".bpm = " << track.bpm << ", "
       << ".bitrate = " << track.bitrate << ", "
       << ".playCount = " << track.playCount << ", "
       << ".lastPlayedAt = " << track.lastPlayedAt.toString().toStdString() << ", "
       << ".dateAdded = " << track.dateAdded.toString().toStdString() << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ITunesPlaylist& playlist) {
    os << "ITunesPlaylist { "
       << ".id = " << playlist.id << ", "
       << ".name = \"" << playlist.name.toStdString() << "\" }";
    return os;
}

void ITunesDAO::initialize(const QSqlDatabase& database) {
    m_insertTrackQuery = QSqlQuery(database);
    m_insertPlaylistQuery = QSqlQuery(database);
    m_insertPlaylistTrackQuery = QSqlQuery(database);
    m_applyPathMappingQuery = QSqlQuery(database);

    m_insertTrackQuery.prepare(
            "INSERT INTO itunes_library (id, artist, title, album, "
            "album_artist, genre, grouping, year, duration, "
            "location, rating, comment, tracknumber, bpm, bitrate, composer, "
            "timesplayed, last_played_at, datetime_added) "
            "VALUES (:id, :artist, :title, :album, :album_artist, "
            ":genre, :grouping, :year, :duration, :location, "
            ":rating, :comment, :tracknumber, :bpm, :bitrate, :composer, "
            ":timesplayed, :last_played_at, :datetime_added)");

    m_insertPlaylistQuery.prepare(
            "INSERT INTO itunes_playlists (id, name, display_name) VALUES "
            "(:id, :name, :display_name)");

    m_insertPlaylistTrackQuery.prepare(
            "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, "
            "position) VALUES (:playlist_id, :track_id, :position)");

    m_applyPathMappingQuery.prepare(
            "UPDATE itunes_library SET location = replace( location, "
            ":itunes_path, :mixxx_path )");

    m_isDatabaseInitialized = true;
}

bool ITunesDAO::importTrack(const ITunesTrack& track) {
    if (m_isDatabaseInitialized) {
        QSqlQuery& query = m_insertTrackQuery;

        query.bindValue(":id", track.id);
        query.bindValue(":artist", track.artist);
        query.bindValue(":title", track.title);
        query.bindValue(":album", track.album);
        query.bindValue(":album_artist", track.albumArtist);
        query.bindValue(":genre", track.genre);
        query.bindValue(":grouping", track.grouping);
        query.bindValue(":year", track.year > 0 ? QVariant(track.year) : QVariant());
        query.bindValue(":duration", track.duration);
        query.bindValue(":location", track.location);
        query.bindValue(":rating", track.rating);
        query.bindValue(":comment", track.comment);
        query.bindValue(":tracknumber",
                track.trackNumber > 0 ? QVariant(track.trackNumber) : QVariant());
        query.bindValue(":bpm", track.bpm);
        query.bindValue(":bitrate", track.bitrate);
        query.bindValue(":composer", track.composer);
        query.bindValue(":timesplayed", track.playCount);
        query.bindValue(":last_played_at", track.lastPlayedAt);
        query.bindValue(":datetime_added", track.dateAdded);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    return true;
}

bool ITunesDAO::importPlaylist(const ITunesPlaylist& playlist) {
    QString uniqueName = uniquifyPlaylistName(playlist.name);

    if (m_isDatabaseInitialized) {
        QSqlQuery& query = m_insertPlaylistQuery;

        query.bindValue(":id", playlist.id);
        query.bindValue(":name", uniqueName);
        query.bindValue(":display_name", playlist.name);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    m_playlistNameById[playlist.id] = playlist.name;

    return true;
}

bool ITunesDAO::importPlaylistRelation(int parentId, int childId) {
    m_playlistIdsByParentId.insert({parentId, childId});
    return true;
}

bool ITunesDAO::importPlaylistTrack(int playlistId, int trackId, int position) {
    if (m_isDatabaseInitialized) {
        QSqlQuery& query = m_insertPlaylistTrackQuery;

        query.bindValue(":playlist_id", playlistId);
        query.bindValue(":track_id", trackId);
        query.bindValue(":position", position);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    return true;
}

bool ITunesDAO::applyPathMapping(const ITunesPathMapping& pathMapping) {
    if (m_isDatabaseInitialized) {
        QSqlQuery& query = m_insertPlaylistTrackQuery;

        query.bindValue(":itunes_path",
                QString(pathMapping.dbITunesRoot).replace(kiTunesLocalhostToken, ""));
        query.bindValue(":mixxx_path", pathMapping.mixxxITunesRoot);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    return true;
}

void ITunesDAO::appendPlaylistTree(gsl::not_null<TreeItem*> item, int playlistId) {
    auto childsRange = m_playlistIdsByParentId.equal_range(playlistId);
    std::for_each(childsRange.first,
            childsRange.second,
            [this, &item](auto childEntry) {
                int childId = childEntry.second;
                QString childName = m_playlistNameById[childId];
                TreeItem* child = item->appendChild(childName, childId);
                appendPlaylistTree(child, childId);
            });
}

QString ITunesDAO::uniquifyPlaylistName(QString name) {
    // iTunes permits users to have multiple playlists with the same name,
    // our data model (both the database schema and the tree items) however
    // require unique names since they identify the playlist via this name.
    // We therefore keep track of duplicates and append a suffix
    // accordingly. E.g. if the user has three playlists named 'House' in
    // their library, the playlists would get named (in this order):
    //
    //     House
    //     House #2
    //     House #3
    //

    // Avoid empty playlist names
    if (name.isEmpty()) {
        name = QObject::tr("(empty)");
    }

    auto existing = m_playlistDuplicatesByName.find(name);
    if (existing != m_playlistDuplicatesByName.end()) {
        m_playlistDuplicatesByName[name] += 1;
        return QString("%1 #%2").arg(name).arg(
                m_playlistDuplicatesByName[name] + 1);
    } else {
        m_playlistDuplicatesByName[name] = 0;
        return name;
    }
}
