#include "library/itunes/itunesmacosimporter.h"

#import <iTunesLibrary/iTunesLibrary.h>

#include <QHash>
#include <QMultiHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <memory>
#include <optional>
#include <utility>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"

namespace {

class ImporterImpl {
   public:
    ImporterImpl(QSqlDatabase& database, bool& cancelImport)
        : m_database(database),
          m_cancelImport(cancelImport) {}

    void importPlaylists(NSArray<ITLibPlaylist*>* playlists) {
        QSqlQuery queryInsertToPlaylists(m_database);
        queryInsertToPlaylists.prepare(
            "INSERT INTO itunes_playlists (id, name) VALUES (:id, :name)");

        QSqlQuery queryInsertToPlaylistTracks(m_database);
        queryInsertToPlaylistTracks.prepare(
            "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");

        qDebug() << "Importing playlists via native iTunesLibrary framework";

        for (ITLibPlaylist* playlist in playlists) {
            if (m_cancelImport) {
                break;
            }

            importPlaylist(playlist, queryInsertToPlaylists,
                           queryInsertToPlaylistTracks);
        }
    }

    void importMediaItems(NSArray<ITLibMediaItem*>* items) {
        QSqlQuery query(m_database);
        query.prepare(
            "INSERT INTO itunes_library (id, artist, title, album, album_artist, "
            "genre, grouping, year, duration, location, rating, comment, "
            "tracknumber, bpm, bitrate) VALUES (:id, :artist, :title, :album, "
            ":album_artist, :genre, :grouping, :year, :duration, :location, "
            ":rating, :comment, :tracknumber, :bpm, :bitrate)");

        qDebug() << "Importing media items via native iTunesLibrary framework";

        for (ITLibMediaItem* item in items) {
            if (m_cancelImport) {
                break;
            }

            importMediaItem(item, query);
        }
    }

    void appendPlaylistTree(TreeItem& item, int playlistId = -1) {
        for (auto it = m_playlistChildsByDbId.find(playlistId);
             it != m_playlistChildsByDbId.end() && it.key() == playlistId;
             ++it) {
            int childId = it.value();
            QString childName = m_playlistNameByDbId[childId];
            TreeItem* child = item.appendChild(childName);
            appendPlaylistTree(*child, childId);
        }
    }

   private:
    QSqlDatabase& m_database;
    bool& m_cancelImport;

    QHash<unsigned long long, int> m_dbIdByPersistentId;
    QHash<QString, int> m_playlistDuplicatesByName;
    QHash<int, QString> m_playlistNameByDbId;
    QMultiHash<int, int> m_playlistChildsByDbId;

    int persistentIdToDbId(NSNumber* boxedPersistentId) {
        // Map a persistent ID as used by iTunes to an (incrementing) database ID
        // The persistent IDs used by iTunes occasionally exceed signed 64-bit ints,
        // so we cannot use them directly, unfortunately (also we currently use the
        // fact that our deterministic indexing scheme starts at 0 to represent the
        // root of the playlist tree with -1 in appendPlaylistTree).
        unsigned long long persistentId = [boxedPersistentId unsignedLongLongValue];
        auto existing = m_dbIdByPersistentId.find(persistentId);
        if (existing != m_dbIdByPersistentId.end()) {
            return existing.value();
        } else {
            int dbId = m_dbIdByPersistentId.size();
            m_dbIdByPersistentId[persistentId] = dbId;
            return dbId;
        }
    }

    QString uniquifyPlaylistName(QString name) {
        // Avoid empty playlist names
        if (name.isEmpty()) {
            name = "(empty)";
        }

        auto existing = m_playlistDuplicatesByName.find(name);
        if (existing != m_playlistDuplicatesByName.end()) {
            m_playlistDuplicatesByName[name] += 1;
            return QString("%1 #%2").arg(name).arg(m_playlistDuplicatesByName[name]);
        } else {
            m_playlistDuplicatesByName[name] = 1;
            return name;
        }
    }

    bool isPlaylistShown(ITLibPlaylist* playlist) {
        // Filter out the primary playlist (since we already show the library
        // under the main iTunes node)
        bool isPrimary;
        if (@available(macOS 12.0, *)) {
            isPrimary = [playlist isPrimary];
        } else {
            isPrimary = [playlist isMaster];
        }
        if (isPrimary) {
            return false;
        }

        // Filter out automatically generated 'category' playlists,
        // most of these are either redundant (e.g. the 'Music' playlist)
        // or include non-audio content.
        switch (playlist.distinguishedKind) {
            case ITLibDistinguishedPlaylistKindMusic:
            case ITLibDistinguishedPlaylistKindMovies:
            case ITLibDistinguishedPlaylistKindPodcasts:
            case ITLibDistinguishedPlaylistKindAudiobooks:
            case ITLibDistinguishedPlaylistKindTVShows:
            case ITLibDistinguishedPlaylistKindiTunesU:
            case ITLibDistinguishedPlaylistKindMusicVideos:
            case ITLibDistinguishedPlaylistKindLibraryMusicVideos:
            case ITLibDistinguishedPlaylistKindHomeVideos:
            case ITLibDistinguishedPlaylistKindApplications:
            case ITLibDistinguishedPlaylistKindMusicShowsAndMovies:
                return false;
            default:
                break;
        }

        return true;
    }

    void importPlaylist(ITLibPlaylist* playlist, QSqlQuery& queryInsertToPlaylists,
                        QSqlQuery& queryInsertToPlaylistTracks) {
        if (!isPlaylistShown(playlist)) {
            return;
        }

        int playlistId = persistentIdToDbId(playlist.persistentID);
        int parentId = playlist.parentID ? persistentIdToDbId(playlist.parentID) : -1;

        QString playlistName = uniquifyPlaylistName(
            [playlist.name cStringUsingEncoding:NSUTF8StringEncoding]);

        queryInsertToPlaylists.bindValue(":id", playlistId);
        queryInsertToPlaylists.bindValue(":name", playlistName);

        if (!queryInsertToPlaylists.exec()) {
            LOG_FAILED_QUERY(queryInsertToPlaylists);
            return;
        }

        m_playlistNameByDbId.insert(playlistId, playlistName);
        m_playlistChildsByDbId.insert(parentId, playlistId);

        int i = 0;
        for (ITLibMediaItem* item in playlist.items) {
            if (m_cancelImport) {
                return;
            }

            queryInsertToPlaylistTracks.bindValue(":playlist_id", playlistId);
            queryInsertToPlaylistTracks.bindValue(
                ":track_id", persistentIdToDbId(item.persistentID));
            queryInsertToPlaylistTracks.bindValue(":position", i);

            if (!queryInsertToPlaylistTracks.exec()) {
                LOG_FAILED_QUERY(queryInsertToPlaylistTracks);
                return;
            }

            i++;
        }
    }

    void importMediaItem(ITLibMediaItem* item, QSqlQuery& query) {
        query.bindValue(":id", persistentIdToDbId(item.persistentID));
        query.bindValue(":artist", [item.artist.name cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":title", [item.title cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":album", [item.album.title cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(
            ":album_artist",
            [item.album.albumArtist cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":genre", [item.genre cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":grouping", [item.grouping cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":year", static_cast<int>(item.year));
        query.bindValue(":duration", static_cast<int>(item.totalTime / 1000));
        query.bindValue(
            ":location",
            [[item.location path] cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":rating", static_cast<int>(item.rating / 20));
        query.bindValue(":comment", [item.comments cStringUsingEncoding:NSUTF8StringEncoding]);
        query.bindValue(":tracknumber", static_cast<int>(item.trackNumber));
        query.bindValue(":bpm", static_cast<int>(item.beatsPerMinute));
        query.bindValue(":bitrate", static_cast<int>(item.bitrate));

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }
    }
};

}  // anonymous namespace

ITunesMacOSImporter::ITunesMacOSImporter(LibraryFeature* parentFeature,
                                         QSqlDatabase& database,
                                         bool& cancelImport)
    : m_parentFeature(parentFeature),
      m_database(database),
      m_cancelImport(cancelImport) {
}

ITunesImport ITunesMacOSImporter::importLibrary() {
    @autoreleasepool {
        ITunesImport iTunesImport;
        iTunesImport.isMusicFolderLocatedAfterTracks = false;

        NSError* error = nil;
        ITLibrary* library = [[ITLibrary alloc] initWithAPIVersion:@"1.0" error:&error];

        if (library) {
            std::unique_ptr<TreeItem> rootItem = TreeItem::newRoot(m_parentFeature);
            ImporterImpl impl(m_database, m_cancelImport);

            impl.importPlaylists(library.allPlaylists);
            impl.importMediaItems(library.allMediaItems);
            impl.appendPlaylistTree(*rootItem);

            iTunesImport.playlistRoot = std::move(rootItem);
        }

        return iTunesImport;
    }
}
