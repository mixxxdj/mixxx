#include "library/playlisttablemodel.h"
#include "library/queryutil.h"
#include "playermanager.h"

PlaylistTableModel::PlaylistTableModel(QObject* parent,
                                       TrackCollection* pTrackCollection,
                                       const char* settingsNamespace,
                                       bool showAll)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace),
          m_iPlaylistId(-1),
          m_showAll(showAll) {
}

PlaylistTableModel::~PlaylistTableModel() {
}

// Must be called from TrackCollection thread
void PlaylistTableModel::setTableModel(int playlistId) {
    // here uses callSync
    //qDebug() << "PlaylistTableModel::setTableModel" << playlistId;
    if (m_iPlaylistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    m_iPlaylistId = playlistId;
    QString playlistTableName = "playlist_" + QString::number(m_iPlaylistId);

    QStringList columns = QStringList()
            << PLAYLISTTRACKSTABLE_TRACKID + " AS " + LIBRARYTABLE_ID
            << PLAYLISTTRACKSTABLE_POSITION
            << PLAYLISTTRACKSTABLE_DATETIMEADDED
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover hash.
            << LIBRARYTABLE_COVERART_HASH + " AS " + LIBRARYTABLE_COVERART;

    m_pTrackCollection->callSync(
                [this, &playlistTableName, &playlistId, &columns](TrackCollectionPrivate* pTrackCollectionPrivate) {
        QSqlQuery query(pTrackCollectionPrivate->getDatabase());
        FieldEscaper escaper(pTrackCollectionPrivate->getDatabase());


        // We drop files that have been explicitly deleted from mixxx
        // (mixxx_deleted=0) from the view. There was a bug in <= 1.9.0 where
        // removed files were not removed from playlists, so some users will have
        // libraries where this is the case.
        QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                      "SELECT %2 FROM PlaylistTracks "
                                      "INNER JOIN library ON library.id = PlaylistTracks.track_id "
                                      "WHERE PlaylistTracks.playlist_id = %3")
                .arg(escaper.escapeString(playlistTableName),
                     columns.join(","),
                     QString::number(playlistId));
        if (!m_showAll) {
            queryString.append(" AND library.mixxx_deleted = 0");
        }
        query.prepare(queryString);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }, __PRETTY_FUNCTION__);

    columns[0] = LIBRARYTABLE_ID;
    columns[3] = LIBRARYTABLE_PREVIEW;
    columns[4] = LIBRARYTABLE_COVERART;
    setTable(playlistTableName, columns[0], columns,
            m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());

    m_pTrackCollection->callSync(
                [this](TrackCollectionPrivate* pTrackCollectionPrivate) {
        connect(&pTrackCollectionPrivate->getPlaylistDAO(), SIGNAL(changed(int)),
                this, SLOT(playlistChanged(int)),Qt::QueuedConnection);
    }, __PRETTY_FUNCTION__);
}

// Must be called from Main thread
int PlaylistTableModel::addTracks(const QModelIndex& index,
                                  const QList<QString>& locations) {
    if (locations.isEmpty()) {
        return 0;
    }

    const int positionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumn).data().toInt();

    // Handle weird cases like a drag and drop to an invalid index
    if (position <= 0) {
        position = rowCount() + 1;
    }

    QList<QFileInfo> fileInfoList;
    foreach (QString fileLocation, locations) {
        QFileInfo fileInfo(fileLocation);
        if (fileInfo.exists()) {
            fileInfoList.append(fileInfo);
        }
    }

    int tracksAdded = 0;
    // tro's lambda idea. This code calls Synchronously!
    m_pTrackCollection->callSync(
                [this, &fileInfoList, &position, &locations, &tracksAdded] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        QList<int> trackIds = pTrackCollectionPrivate->getTrackDAO().addTracks(fileInfoList, true);

        tracksAdded = pTrackCollectionPrivate->getPlaylistDAO().insertTracksIntoPlaylist(
                    trackIds, m_iPlaylistId, position);

        if (tracksAdded > 0) {
            select(pTrackCollectionPrivate);
        } else if (locations.size() - tracksAdded > 0) {
            qDebug() << "PlaylistTableModel::addTracks could not add"
                     << locations.size() - tracksAdded
                     << "to playlist" << m_iPlaylistId;
        }
    }, __PRETTY_FUNCTION__);
    return tracksAdded;
}

// Must be called from Main
bool PlaylistTableModel::appendTrack(const int trackId) {
    if (trackId < 0) {
        return false;
    }
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
               [this, trackId] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        pTrackCollectionPrivate->getPlaylistDAO().appendTrackToPlaylist(trackId, m_iPlaylistId);
        select(pTrackCollectionPrivate); //Repopulate the data model.
    }, __PRETTY_FUNCTION__);
    return true;
}

// Must be called from Main
void PlaylistTableModel::removeTrack(const QModelIndex& index) {
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, index] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        if (pTrackCollectionPrivate->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
            return;
        }

        const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
        const int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
        if (pTrackCollectionPrivate->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
            return;
        }
        pTrackCollectionPrivate->getPlaylistDAO().removeTrackFromPlaylist(m_iPlaylistId, position);
        select(pTrackCollectionPrivate); // Repopulate the data model.
    }, __PRETTY_FUNCTION__);
}

void PlaylistTableModel::removeTracks(const QModelIndexList& indices) {
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, indices] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        bool locked = pTrackCollectionPrivate->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);

        if (locked) {
            return;
        }
        const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

        QList<int> trackPositions;
        foreach (QModelIndex index, indices) {
            int trackPosition = index.sibling(index.row(), positionColumnIndex).data().toInt();
            trackPositions.append(trackPosition);
        }
        pTrackCollectionPrivate->getPlaylistDAO().removeTracksFromPlaylist(m_iPlaylistId,trackPositions);
    });
}

void PlaylistTableModel::moveTrack(const QModelIndex& sourceIndex,
                                   const QModelIndex& destIndex) {

    int playlistPositionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    // this->record(destIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();
    int newPosition = destIndex.sibling(destIndex.row(), playlistPositionColumn).data().toInt();
    // this->record(sourceIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();
    const int oldPosition = sourceIndex.sibling(sourceIndex.row(), playlistPositionColumn).data().toInt();

    if (newPosition > oldPosition) {
        // new position moves up due to closing the gap of the old position
        --newPosition;
    }

    //qDebug() << "old pos" << oldPosition << "new pos" << newPosition;
    if (newPosition < 0 || newPosition == oldPosition) {
        // Invalid for the position to be 0 or less.
        // or no move at all
        return;
    } else if (newPosition == 0) {
        //Dragged out of bounds, which is past the end of the rows...
        newPosition = rowCount();
    }
    m_pTrackCollection->callSync(
                [this, &oldPosition, &newPosition](TrackCollectionPrivate* pTrackCollectionPrivate) {
        pTrackCollectionPrivate->getPlaylistDAO().moveTrack(m_iPlaylistId, oldPosition, newPosition);
    }, __PRETTY_FUNCTION__);
}

bool PlaylistTableModel::isLocked() {
    bool locked = false;
    // tro's lambda idea. This code calls synchronously!
     m_pTrackCollection->callSync(
                 [this, &locked] (TrackCollectionPrivate* pTrackCollectionPrivate) {
         locked = pTrackCollectionPrivate->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
     }, __PRETTY_FUNCTION__);
    return locked;
}

void PlaylistTableModel::shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude) {
    QList<int> positions;
    QHash<int,int> allIds;
    const int positionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    const int idColumn = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID);
    int excludePos = -1;
    if (exclude.row() > -1) {
        // this is used to exclude the already loaded track at pos #1 if used from running Auto-DJ
        excludePos = exclude.sibling(exclude.row(), positionColumn).data().toInt();
    }
    if (shuffle.count() > 1) {
        // if there is more then one track selected, shuffle selection only
        foreach(QModelIndex shuffleIndex, shuffle) {
            int oldPosition = shuffleIndex.sibling(shuffleIndex.row(), positionColumn).data().toInt();
            if (oldPosition != excludePos) {
                positions.append(oldPosition);
            }
        }
    } else {
        // if there is only one track selected, shuffle all tracks
        int numOfTracks = rowCount();
        for (int i = 0; i < numOfTracks; i++) {
            int oldPosition = index(i, positionColumn).data().toInt();
            if (oldPosition != excludePos) {
                positions.append(oldPosition);
            }
        }
    }

    // Set up list of all IDs
    int numOfTracks = rowCount();
    for (int i = 0; i < numOfTracks; i++) {
        int position = index(i, positionColumn).data().toInt();
        int id = index(i, idColumn).data().toInt();
        allIds.insert(position, id);
    }
    m_pTrackCollection->callSync(
                 [this, &positions, &allIds] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        pTrackCollectionPrivate->getPlaylistDAO().shuffleTracks(m_iPlaylistId, positions, allIds);
    }, __PRETTY_FUNCTION__);
}

bool PlaylistTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID) ||
            column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID)||
            column == fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_FSDELETED) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH)) {
        return true;
    }
    return false;
}

bool PlaylistTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED)) {
        return true;
    }
    return false;
}

TrackModel::CapabilitiesFlags PlaylistTableModel::getCapabilities() const {
    TrackModel::CapabilitiesFlags caps = TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_RECEIVEDROPS
            | TRACKMODELCAPS_REORDER
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_RELOADMETADATA
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_REMOVE
            | TRACKMODELCAPS_MANIPULATEBEATS
            | TRACKMODELCAPS_CLEAR_BEATS
            | TRACKMODELCAPS_RESETPLAYED;

    // Only allow Add to AutoDJ if we aren't currently showing the AutoDJ queue.
    m_pTrackCollection->callSync( [this, &caps] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        if (m_iPlaylistId != pTrackCollectionPrivate->getPlaylistDAO().getPlaylistIdFromName(AUTODJ_TABLE)) {
            caps |= TRACKMODELCAPS_ADDTOAUTODJ;
        }
        bool locked = pTrackCollectionPrivate->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
        if (locked) {
            caps |= TRACKMODELCAPS_LOCKED;
        }
    }, __PRETTY_FUNCTION__);

    return caps;
}

void PlaylistTableModel::playlistChanged(int playlistId) {
    if (playlistId == m_iPlaylistId) {
        m_pTrackCollection->callSync(
            [this] (TrackCollectionPrivate* pTrackCollectionPrivate) {
            select(pTrackCollectionPrivate); // Repopulate the data model.
        }, __PRETTY_FUNCTION__);
    }
}
