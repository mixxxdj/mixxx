#include "library/itunes/itunesimportbackend.h"

#include <QSqlQuery>

#include "library/itunes/ituneslocalhosttoken.h"
#include "library/queryutil.h"

ITunesImportBackend::ITunesImportBackend(const QSqlDatabase& database)
        : m_insertTrackQuery(database),
          m_insertPlaylistQuery(database),
          m_insertPlaylistTrackQuery(database),
          m_applyPathMappingQuery(database) {
    m_insertTrackQuery.prepare(
            "INSERT INTO itunes_library (id, artist, title, album, "
            "album_artist, genre, grouping, year, duration, "
            "location, rating, comment, tracknumber, bpm, bitrate) "
            "VALUES (:id, :artist, :title, :album, :album_artist, "
            ":genre, :grouping, :year, :duration, :location, "
            ":rating, :comment, :tracknumber, :bpm, :bitrate)");

    m_insertPlaylistQuery.prepare("INSERT INTO itunes_playlists (id, name) VALUES (:id, :name)");

    m_insertPlaylistTrackQuery.prepare(
            "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, "
            "position) VALUES (:playlist_id, :track_id, :position)");

    m_applyPathMappingQuery.prepare(
            "UPDATE itunes_library SET location = replace( location, "
            ":itunes_path, :mixxx_path )");
}

bool ITunesImportBackend::importTrack(ITunesTrack track) {
    QSqlQuery& query = m_insertTrackQuery;

    query.bindValue(":id", track.id);
    query.bindValue(":artist", track.artist);
    query.bindValue(":title", track.title);
    query.bindValue(":album", track.album);
    query.bindValue(":album_artist", track.albumArtist);
    query.bindValue(":genre", track.genre);
    query.bindValue(":grouping", track.grouping);
    query.bindValue(":duration", track.duration);
    query.bindValue(":location", track.location);
    query.bindValue(":rating", track.rating);
    query.bindValue(":comment", track.comment);
    if (track.year > 0) {
        query.bindValue(":year", track.year);
    }
    if (track.trackNumber > 0) {
        query.bindValue(":tracknumber", track.trackNumber);
    }
    if (track.bpm > 0) {
        query.bindValue(":bpm", track.bpm);
    }
    if (track.bitrate > 0) {
        query.bindValue(":bitrate", track.bitrate);
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool ITunesImportBackend::importPlaylist(ITunesPlaylist playlist) {
    playlist.name = uniquifyPlaylistName(playlist.name);

    QSqlQuery& query = m_insertPlaylistQuery;

    query.bindValue(":id", playlist.id);
    query.bindValue(":name", playlist.name);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    m_playlistNameById[playlist.id] = playlist.name;

    return true;
}

bool ITunesImportBackend::importPlaylistRelation(int parentId, int childId) {
    m_playlistIdsByParentId.insert({parentId, childId});
    return true;
}

bool ITunesImportBackend::importPlaylistTrack(int playlistId, int trackId, int position) {
    QSqlQuery& query = m_insertPlaylistTrackQuery;

    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool ITunesImportBackend::applyPathMapping(ITunesPathMapping pathMapping) {
    QSqlQuery& query = m_insertPlaylistTrackQuery;

    query.bindValue(":itunes_path",
            pathMapping.dbITunesRoot.replace(kiTunesLocalhostToken, ""));
    query.bindValue(":mixxx_path", pathMapping.mixxxITunesRoot);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

void ITunesImportBackend::appendPlaylistTree(TreeItem& item, int playlistId) {
    auto childsRange = m_playlistIdsByParentId.equal_range(playlistId);
    std::for_each(childsRange.first,
            childsRange.second,
            [this, &item](auto childEntry) {
                int childId = childEntry.second;
                QString childName = m_playlistNameById[childId];
                TreeItem* child = item.appendChild(childName);
                appendPlaylistTree(*child, childId);
            });
}

QString ITunesImportBackend::uniquifyPlaylistName(QString name) {
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
        name = "(empty)";
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
