#include "library/itunes/itunesmacosimporter.h"

#import <iTunesLibrary/iTunesLibrary.h>

#include <QHash>
#include <QMultiHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <atomic>
#include <memory>
#include <optional>
#include <utility>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"

namespace {

QString qStringFrom(NSString* nsString) {
    return [nsString cStringUsingEncoding:NSUTF8StringEncoding];
}

class ImporterImpl {
  public:
    ImporterImpl(QSqlDatabase& database, std::atomic<bool>& cancelImport)
            : m_database(database), m_cancelImport(cancelImport) {
    }

    void importPlaylists(NSArray<ITLibPlaylist*>* playlists) {
        QSqlQuery queryInsertToPlaylists(m_database);
        queryInsertToPlaylists.prepare(
                "INSERT INTO itunes_playlists (id, name) VALUES (:id, :name)");

        QSqlQuery queryInsertToPlaylistTracks(m_database);
        queryInsertToPlaylistTracks.prepare(
                "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, "
                "position) VALUES (:playlist_id, :track_id, :position)");

        qDebug() << "Importing playlists via native iTunesLibrary framework";

        for (ITLibPlaylist* playlist in playlists) {
            if (m_cancelImport) {
                break;
            }

            importPlaylist(playlist,
                    queryInsertToPlaylists,
                    queryInsertToPlaylistTracks);
        }
    }

    void importMediaItems(NSArray<ITLibMediaItem*>* items) {
        QSqlQuery query(m_database);
        query.prepare("INSERT INTO itunes_library (id, artist, title, album, "
                      "album_artist, genre, grouping, year, duration, "
                      "location, rating, comment, tracknumber, bpm, bitrate) "
                      "VALUES (:id, :artist, :title, :album, :album_artist, "
                      ":genre, :grouping, :year, :duration, :location, "
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
    std::atomic<bool>& m_cancelImport;

    QHash<unsigned long long, int> m_dbIdByPersistentId;
    QHash<QString, int> m_playlistDuplicatesByName;
    QHash<int, QString> m_playlistNameByDbId;
    QMultiHash<int, int> m_playlistChildsByDbId;

    int dbIdFromPersistentId(NSNumber* boxedPersistentId) {
        // Map a persistent ID as used by iTunes to an (incrementing) database
        // ID The persistent IDs used by iTunes occasionally exceed signed
        // 64-bit ints, so we cannot use them directly, unfortunately (also we
        // currently use the fact that our deterministic indexing scheme starts
        // at 0 to represent the root of the playlist tree with -1 in
        // appendPlaylistTree).
        unsigned long long persistentId =
                [boxedPersistentId unsignedLongLongValue];
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

    void importPlaylist(ITLibPlaylist* playlist,
            QSqlQuery& queryInsertToPlaylists,
            QSqlQuery& queryInsertToPlaylistTracks) {
        if (!isPlaylistShown(playlist)) {
            return;
        }

        int playlistId = dbIdFromPersistentId(playlist.persistentID);
        int parentId = playlist.parentID
                ? dbIdFromPersistentId(playlist.parentID)
                : -1;

        QString playlistName = uniquifyPlaylistName(qStringFrom(playlist.name));

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
                    ":track_id", dbIdFromPersistentId(item.persistentID));
            queryInsertToPlaylistTracks.bindValue(":position", i);

            if (!queryInsertToPlaylistTracks.exec()) {
                LOG_FAILED_QUERY(queryInsertToPlaylistTracks);
                return;
            }

            i++;
        }
    }

    void importMediaItem(ITLibMediaItem* item, QSqlQuery& query) {
        query.bindValue(":id", dbIdFromPersistentId(item.persistentID));
        query.bindValue(":artist", qStringFrom(item.artist.name));
        query.bindValue(":title", qStringFrom(item.title));
        query.bindValue(":album", qStringFrom(item.album.title));
        query.bindValue(":album_artist", qStringFrom(item.album.albumArtist));
        query.bindValue(":genre", qStringFrom(item.genre));
        query.bindValue(":grouping", qStringFrom(item.grouping));
        query.bindValue(":year", static_cast<int>(item.year));
        query.bindValue(":duration", static_cast<int>(item.totalTime / 1000));
        query.bindValue(":location", qStringFrom([item.location path]));
        query.bindValue(":rating", static_cast<int>(item.rating / 20));
        query.bindValue(":comment", qStringFrom(item.comments));
        query.bindValue(":tracknumber", static_cast<int>(item.trackNumber));
        query.bindValue(":bpm", static_cast<int>(item.beatsPerMinute));
        query.bindValue(":bitrate", static_cast<int>(item.bitrate));

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }
    }
};

} // anonymous namespace

ITunesMacOSImporter::ITunesMacOSImporter(LibraryFeature* parentFeature,
        QSqlDatabase& database,
        std::atomic<bool>& cancelImport)
        : m_parentFeature(parentFeature),
          m_database(database),
          m_cancelImport(cancelImport) {
}

ITunesImport ITunesMacOSImporter::importLibrary() {
    @autoreleasepool {
        ITunesImport iTunesImport;
        iTunesImport.isMusicFolderLocatedAfterTracks = false;

        NSError* error = nil;
        ITLibrary* library = [[ITLibrary alloc] initWithAPIVersion:@"1.0"
                                                             error:&error];

        if (library) {
            std::unique_ptr<TreeItem> rootItem =
                    TreeItem::newRoot(m_parentFeature);
            ImporterImpl impl(m_database, m_cancelImport);

            impl.importPlaylists(library.allPlaylists);
            impl.importMediaItems(library.allMediaItems);
            impl.appendPlaylistTree(*rootItem);

            iTunesImport.playlistRoot = std::move(rootItem);
        }

        return iTunesImport;
    }
}
