#include "library/itunes/itunesmacosimporter.h"

#import <iTunesLibrary/iTunesLibrary.h>
#include "library/itunes/itunesdao.h"

#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <algorithm>
#include <atomic>
#include <map>
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
    ImporterImpl(const std::atomic<bool>& cancelImport, ITunesDAO& dao)
            : m_cancelImport(cancelImport), m_dao(dao) {
    }

    void importPlaylists(NSArray<ITLibPlaylist*>* playlists) {
        qDebug() << "Importing playlists via native iTunesLibrary framework";

        // We prefer Objective-C-style for-in loops over C++ loops when dealing
        // with Objective-C types (both here and in the methods below) since
        // they use Objective-C's enumeration protocols and are guaranteed to
        // interact well with Objective-C collections.

        for (ITLibPlaylist* playlist in playlists) {
            if (m_cancelImport.load()) {
                break;
            }

            importPlaylist(playlist);
        }
    }

    void importMediaItems(NSArray<ITLibMediaItem*>* items) {
        qDebug() << "Importing media items via native iTunesLibrary framework";

        for (ITLibMediaItem* item in items) {
            if (m_cancelImport.load()) {
                break;
            }

            importMediaItem(item);
        }
    }

    void appendPlaylistTree(TreeItem& item) {
        m_dao.appendPlaylistTree(item);
    }

  private:
    const std::atomic<bool>& m_cancelImport;

    QHash<unsigned long long, int> m_dbIdByPersistentId;
    ITunesDAO& m_dao;

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

    void importPlaylist(ITLibPlaylist* itPlaylist) {
        if (!isPlaylistShown(itPlaylist)) {
            return;
        }

        int playlistId = dbIdFromPersistentId(itPlaylist.persistentID);
        int parentId = itPlaylist.parentID
                ? dbIdFromPersistentId(itPlaylist.parentID)
                : kRootITunesPlaylistId;

        ITunesPlaylist playlist = {};
        playlist.id = playlistId;
        playlist.name = qStringFrom(itPlaylist.name);
        if (!m_dao.importPlaylist(playlist)) {
            return;
        }

        if (!m_dao.importPlaylistRelation(parentId, playlistId)) {
            return;
        }

        int i = 0;
        for (ITLibMediaItem* item in itPlaylist.items) {
            if (m_cancelImport.load()) {
                return;
            }

            int trackId = dbIdFromPersistentId(item.persistentID);
            if (!m_dao.importPlaylistTrack(playlistId, trackId, i)) {
                return;
            }

            i++;
        }
    }

    void importMediaItem(ITLibMediaItem* item) {
        // Skip DRM-protected and non-downloaded tracks
        bool isRemote = item.locationType == ITLibMediaItemLocationTypeRemote;
        if (item.drmProtected || isRemote) {
            return;
        }

        ITunesTrack track = {};
        track.id = dbIdFromPersistentId(item.persistentID);
        track.artist = qStringFrom(item.artist.name);
        track.title = qStringFrom(item.title);
        track.album = qStringFrom(item.album.title);
        track.albumArtist = qStringFrom(item.album.albumArtist);
        track.genre = qStringFrom(item.genre);
        track.grouping = qStringFrom(item.grouping);
        track.year = static_cast<int>(item.year);
        track.duration = static_cast<int>(item.totalTime / 1000);
        track.location = qStringFrom([item.location path]);
        track.rating = static_cast<int>(item.rating / 20);
        track.comment = qStringFrom(item.comments);
        track.trackNumber = static_cast<int>(item.trackNumber);
        track.bpm = static_cast<int>(item.beatsPerMinute);
        track.bitrate = static_cast<int>(item.bitrate);

        if (!m_dao.importTrack(track)) {
            return;
        }
    }
};

} // anonymous namespace

ITunesMacOSImporter::ITunesMacOSImporter(LibraryFeature* parentFeature,
        const std::atomic<bool>& cancelImport,
        ITunesDAO& dao)
        : m_parentFeature(parentFeature),
          m_cancelImport(cancelImport),
          m_dao(dao) {
}

ITunesImport ITunesMacOSImporter::importLibrary() {
    ITunesImport iTunesImport;

    NSError* error = nil;
    ITLibrary* library = [[ITLibrary alloc] initWithAPIVersion:@"1.0"
                                                         error:&error];

    if (library) {
        std::unique_ptr<TreeItem> rootItem = TreeItem::newRoot(m_parentFeature);
        ImporterImpl impl(m_cancelImport, m_dao);

        impl.importPlaylists(library.allPlaylists);
        impl.importMediaItems(library.allMediaItems);
        impl.appendPlaylistTree(*rootItem);

        iTunesImport.playlistRoot = std::move(rootItem);
    } else if (error) {
        qWarning() << "Error reading default iTunes library: "
                   << qStringFrom([error localizedDescription]);
    }

    return iTunesImport;
}
