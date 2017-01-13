#include <QString>

#include "library/features/playlist/playlisttablemodel.h"
#include "library/queryutil.h"
#include "mixer/playermanager.h"

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

void PlaylistTableModel::setTableModel(int playlistId) {
    m_iPlaylistId = playlistId;
    QSet<int> temp;
    temp.insert(playlistId);
    setTableModel(temp);
}

void PlaylistTableModel::setTableModel(const QSet<int> &playlistIds) {
    if (m_playlistIds == playlistIds) {
        qDebug() << "Already focused on playlist " << playlistIds;
        return;
    }
    
    if (playlistIds.size() > 1) {
        // If we are showing many playlist at once this is not a real playlist
        m_iPlaylistId = -1;
    } else {
        m_iPlaylistId = *playlistIds.begin();
    }

    m_playlistIds = playlistIds;
    
    QString playlistTableName = "playlist";
    QStringList sIds;
    for (int n : m_playlistIds) {
        QString sNum = QString::number(n);
        sIds << sNum;
        playlistTableName.append("_" + sNum);
    }
    
    QSqlQuery query(m_database);
    FieldEscaper escaper(m_database);

    QStringList columns;
    columns << PLAYLISTTRACKSTABLE_TRACKID + " AS " + LIBRARYTABLE_ID
            << PLAYLISTTRACKSTABLE_POSITION
            << PLAYLISTTRACKSTABLE_DATETIMEADDED
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover hash.
            << LIBRARYTABLE_COVERART_HASH + " AS " + LIBRARYTABLE_COVERART;

    // We drop files that have been explicitly deleted from mixxx
    // (mixxx_deleted=0) from the view. There was a bug in <= 1.9.0 where
    // removed files were not removed from playlists, so some users will have
    // libraries where this is the case.
    QString queryString = QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                                  "SELECT %2 FROM PlaylistTracks "
                                  "INNER JOIN library ON library.id = PlaylistTracks.track_id "
                                  "WHERE PlaylistTracks.playlist_id IN (%3)")
                          .arg(escaper.escapeString(playlistTableName),
                               columns.join(","),
                               sIds.join(","));
    if (!m_showAll) {
        queryString.append(" AND library.mixxx_deleted = 0");
    }
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    // columns[1] = PLAYLISTTRACKSTABLE_POSITION from above
    // columns[2] = PLAYLISTTRACKSTABLE_DATETIMEADDED from above
    columns[3] = LIBRARYTABLE_PREVIEW;
    columns[4] = LIBRARYTABLE_COVERART;
    setTable(playlistTableName, LIBRARYTABLE_ID, columns,
            m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());

    connect(&m_pTrackCollection->getPlaylistDAO(), SIGNAL(changed(int)),
            this, SLOT(playlistChanged(int)));
}

int PlaylistTableModel::addTracks(const QModelIndex& index,
                                  const QList<QString>& locations) {
    if (locations.isEmpty()) {
        return 0;
    }
    
    int position = getPosition(index);

    // Handle weird cases like a drag and drop to an invalid index
    if (position <= 0) {
        position = rowCount() + 1;
    }

    QList<QFileInfo> fileInfoList;
    for (const QString& fileLocation : locations) {
        QFileInfo fileInfo(fileLocation);
        if (fileInfo.exists()) {
            fileInfoList.append(fileInfo);
        }
    }

    QList<TrackId> trackIds = m_trackDAO.addMultipleTracks(fileInfoList, true);

    int tracksAdded = m_pTrackCollection->getPlaylistDAO().insertTracksIntoPlaylist(
        trackIds, m_iPlaylistId, position);

    if (locations.size() - tracksAdded > 0) {
        qDebug() << "PlaylistTableModel::addTracks could not add"
                 << locations.size() - tracksAdded
                 << "to playlist" << m_iPlaylistId;
    }
    return tracksAdded;
}

bool PlaylistTableModel::appendTrack(TrackId trackId) {
    if (!trackId.isValid()) {
        return false;
    }
    return m_pTrackCollection->getPlaylistDAO().appendTrackToPlaylist(trackId, m_iPlaylistId);
}

void PlaylistTableModel::removeTrack(const QModelIndex& index) {
    if (m_pTrackCollection->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    m_pTrackCollection->getPlaylistDAO()
            .removeTrackFromPlaylist(m_iPlaylistId, getPosition(index));
}

void PlaylistTableModel::removeTracks(const QModelIndexList& indices) {
    if (m_pTrackCollection->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    QList<int> trackPositions;
    for (const QModelIndex& index : indices) {
        trackPositions.append(getPosition(index));
    }

    m_pTrackCollection->getPlaylistDAO()
            .removeTracksFromPlaylist(m_iPlaylistId, trackPositions);
}

void PlaylistTableModel::moveTrack(const QModelIndex& sourceIndex,
                                   const QModelIndex& destIndex) {
    int newPosition = getPosition(destIndex);
    int oldPosition = getPosition(sourceIndex);

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
        // Dragged out of bounds, which is past the end of the rows...
        newPosition = m_pTrackCollection->getPlaylistDAO().getMaxPosition(m_iPlaylistId);
    }

    m_pTrackCollection->getPlaylistDAO().moveTrack(m_iPlaylistId, oldPosition, newPosition);
}

bool PlaylistTableModel::isLocked() {
    return m_pTrackCollection->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
}

void PlaylistTableModel::shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude) {
    QList<int> positions;
    QHash<int,TrackId> allIds;
    int excludePos = -1;
    if (exclude.row() > -1) {
        // this is used to exclude the already loaded track at pos #1 if used from running Auto-DJ
        excludePos = getPosition(exclude);
    }
    if (shuffle.count() > 1) {
        // if there is more then one track selected, shuffle selection only
        for (const QModelIndex& shuffleIndex : shuffle) {
            int oldPosition = getPosition(shuffleIndex);
            if (oldPosition != excludePos) {
                positions.append(oldPosition);
            }
        }
    } else {
        // if there is only one track selected, shuffle all tracks
        int numOfTracks = rowCount();
        for (int i = 0; i < numOfTracks; i++) {
            int oldPosition =  getPosition(index(i, 0));
            if (oldPosition != excludePos) {
                positions.append(oldPosition);
            }
        }
    }
    // Set up list of all IDs
    int numOfTracks = rowCount();
    for (int i = 0; i < numOfTracks; i++) {
        int position = getPosition(index(i, 0));
        TrackId trackId(getTrackId(index(i, 0)));
        allIds.insert(position, trackId);
    }
    m_pTrackCollection->getPlaylistDAO().shuffleTracks(m_iPlaylistId, positions, allIds);
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
    } else if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION)) {
        return false;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
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
    if (m_iPlaylistId != m_pTrackCollection->getPlaylistDAO().getPlaylistIdFromName(AUTODJ_TABLE)) {
        caps |= TRACKMODELCAPS_ADDTOAUTODJ;
    }
    bool locked = m_pTrackCollection->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
    if (locked) {
        caps |= TRACKMODELCAPS_LOCKED;
    }

    return caps;
}

void PlaylistTableModel::saveSelection(const QModelIndexList& selection) {
    m_savedSelectionIndices.clear();
    
    for (const QModelIndex& index : selection) {
        m_savedSelectionIndices.insert(getPosition(index));
    }
}

QModelIndexList PlaylistTableModel::getSavedSelectionIndices() {
    QModelIndexList ret;
    for (const int& pos : m_savedSelectionIndices) {
        auto it = m_positionToRow.find(pos);
        if (it != m_positionToRow.constEnd()) {
            ret << index(*it, 0);
        }
    }
    return ret;
}

void PlaylistTableModel::select() {
    BaseSqlTableModel::select();
    
    m_positionToRow.clear();
    for (int i = 0; i < rowCount(); ++i) {
        int pos = getPosition(index(i, 0));
        m_positionToRow[pos] = i;
    }
}

void PlaylistTableModel::playlistChanged(int playlistId) {
    if (playlistId == m_iPlaylistId) {
        select(); // Repopulate the data model.
    }
}

int PlaylistTableModel::getPosition(const QModelIndex& index) {
    const int positionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    return index.sibling(index.row(), positionColumn).data().toInt();
}
