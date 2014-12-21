#include "library/playlisttablemodel.h"
#include "library/queryutil.h"
#include "playermanager.h"

PlaylistTableModel::PlaylistTableModel(QObject* parent,
                                       TrackCollection* pTrackCollection,
                                       const char* settingsNamespace,
                                       bool showAll)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace),
          m_playlistDao(m_pTrackCollection->getPlaylistDAO()),
          m_iPlaylistId(-1),
          m_showAll(showAll) {
}

PlaylistTableModel::~PlaylistTableModel() {
}

void PlaylistTableModel::setTableModel(int playlistId) {
    //qDebug() << "PlaylistTableModel::setTableModel" << playlistId;
    if (m_iPlaylistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    m_iPlaylistId = playlistId;
    QString playlistTableName = "playlist_" + QString::number(m_iPlaylistId);
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

    columns[0] = LIBRARYTABLE_ID;
    columns[3] = LIBRARYTABLE_PREVIEW;
    columns[4] = LIBRARYTABLE_COVERART;
    setTable(playlistTableName, columns[0], columns,
            m_pTrackCollection->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());

    connect(&m_playlistDao, SIGNAL(changed(int)),
            this, SLOT(playlistChanged(int)));
}

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

    QList<int> trackIds = m_trackDAO.addTracks(fileInfoList, true);

    int tracksAdded = m_playlistDao.insertTracksIntoPlaylist(
        trackIds, m_iPlaylistId, position);

    if (locations.size() - tracksAdded > 0) {
        qDebug() << "PlaylistTableModel::addTracks could not add"
                 << locations.size() - tracksAdded
                 << "to playlist" << m_iPlaylistId;
    }
    return tracksAdded;
}

bool PlaylistTableModel::appendTrack(int trackId) {
    if (trackId < 0) {
        return false;
    }
    return m_playlistDao.appendTrackToPlaylist(trackId, m_iPlaylistId);
}

void PlaylistTableModel::removeTrack(const QModelIndex& index) {
    if (m_playlistDao.isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
    m_playlistDao.removeTrackFromPlaylist(m_iPlaylistId, position);
}

void PlaylistTableModel::removeTracks(const QModelIndexList& indices) {
    if (m_playlistDao.isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    QList<int> trackPositions;
    foreach (QModelIndex index, indices) {
        int trackPosition = index.sibling(index.row(), positionColumnIndex).data().toInt();
        trackPositions.append(trackPosition);
    }

    m_playlistDao.removeTracksFromPlaylist(m_iPlaylistId,trackPositions);
}

void PlaylistTableModel::moveTrack(const QModelIndex& sourceIndex,
                                   const QModelIndex& destIndex) {

    int playlistPositionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    int newPosition = destIndex.sibling(destIndex.row(), playlistPositionColumn).data().toInt();
    int oldPosition = sourceIndex.sibling(sourceIndex.row(), playlistPositionColumn).data().toInt();

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

    m_playlistDao.moveTrack(m_iPlaylistId, oldPosition, newPosition);
}

bool PlaylistTableModel::isLocked() {
    return m_playlistDao.isPlaylistLocked(m_iPlaylistId);
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
    m_playlistDao.shuffleTracks(m_iPlaylistId, positions, allIds);
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
    if (m_iPlaylistId != m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE)) {
        caps |= TRACKMODELCAPS_ADDTOAUTODJ;
    }
    bool locked = m_playlistDao.isPlaylistLocked(m_iPlaylistId);
    if (locked) {
        caps |= TRACKMODELCAPS_LOCKED;
    }

    return caps;
}

void PlaylistTableModel::playlistChanged(int playlistId) {
    if (playlistId == m_iPlaylistId) {
        select(); // Repopulate the data model.
    }
}
