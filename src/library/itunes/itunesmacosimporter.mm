#include "library/itunes/itunesmacosimporter.h"

#import <iTunesLibrary/iTunesLibrary.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <memory>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/treeitemmodel.h"

namespace {

class ImporterImpl {
   public:
    ImporterImpl(LibraryFeature* parentFeature, QSqlDatabase& database, bool& cancelImport)
        : m_parentFeature(parentFeature),
          m_database(database),
          m_cancelImport(cancelImport),
          m_persistentIdToDbId([[NSMutableDictionary alloc] init]) {}

    std::unique_ptr<TreeItem> importPlaylists(NSArray<ITLibPlaylist*>* playlists) {
        std::unique_ptr<TreeItem> rootItem = TreeItem::newRoot(m_parentFeature);

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
                           queryInsertToPlaylistTracks, *rootItem);
        }

        return rootItem;
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

   private:
    LibraryFeature* m_parentFeature;
    QSqlDatabase& m_database;
    bool& m_cancelImport;
    NSMutableDictionary<NSNumber*, NSNumber*>* m_persistentIdToDbId;

    int persistentIdToDbId(NSNumber* persistentId) {
        // TODO: Unfortunately the ids iTunes uses are occasionally larger than 64-bit integers,
        // hand-rolling an indexing scheme however comes with the drawback of instability across
        // executions and may require a lot of space for large libraries. Perhaps some form of
        // hashing would be a better option with some strategy to avoid collisions?
        NSNumber* existing = m_persistentIdToDbId[persistentId];
        if (existing) {
            return [existing intValue];
        } else {
            int dbId = [m_persistentIdToDbId count];
            m_persistentIdToDbId[persistentId] = [NSNumber numberWithInt:dbId];
            return dbId;
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

        // Filter out folders for now
        switch (playlist.kind) {
            case ITLibPlaylistKindFolder:
                return false;
            default:
                break;
        }

        // Filter out non-audio content
        switch (playlist.distinguishedKind) {
            case ITLibDistinguishedPlaylistKindMovies:
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
                        QSqlQuery& queryInsertToPlaylistTracks,
                        TreeItem& treeItem) {
        if (!isPlaylistShown(playlist)) {
            return;
        }

        int playlistId = persistentIdToDbId(playlist.persistentID);
        QString playlistName = [playlist.name cStringUsingEncoding:NSUTF8StringEncoding];

        queryInsertToPlaylists.bindValue(":id", playlistId);
        queryInsertToPlaylists.bindValue(":name", playlistName);

        // TODO: Handle duplicate playlists by appending id (see xml importer)

        if (!queryInsertToPlaylists.exec()) {
            LOG_FAILED_QUERY(queryInsertToPlaylists);
            return;
        }

        treeItem.appendChild(playlistName);

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
            ImporterImpl impl(m_parentFeature, m_database, m_cancelImport);

            iTunesImport.playlistRoot = impl.importPlaylists(library.allPlaylists);
            impl.importMediaItems(library.allMediaItems);
        }

        return iTunesImport;
    }
}
