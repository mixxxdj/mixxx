#include "library/playlisttablemodel.h"

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_playlisttablemodel.cpp"

PlaylistTableModel::PlaylistTableModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace,
        bool keepDeletedTracks)
        : TrackSetTableModel(parent, pTrackCollectionManager, settingsNamespace),
          m_iPlaylistId(-1),
          m_keepDeletedTracks(keepDeletedTracks) {
    connect(&m_pTrackCollectionManager->internalCollection()->getPlaylistDAO(),
            &PlaylistDAO::tracksChanged,
            this,
            &PlaylistTableModel::playlistsChanged);
}

void PlaylistTableModel::initSortColumnMapping() {
    // Add a bijective mapping between the SortColumnIds and column indices
    for (int i = 0; i < static_cast<int>(TrackModel::SortColumnId::IdMax); ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Artist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Title)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Album)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::AlbumArtist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Year)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Genre)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Composer)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Grouping)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::TrackNumber)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::FileType)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::NativeLocation)] =
            fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Comment)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Duration)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::BitRate)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Bpm)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::ReplayGain)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::DateTimeAdded)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::TimesPlayed)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::LastPlayedAt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Rating)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Key)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Preview)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::CoverArt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Position)] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = static_cast<int>(TrackModel::SortColumnId::IdMin);
            i < static_cast<int>(TrackModel::SortColumnId::IdMax);
            ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(
                m_columnIndexBySortColumnId[static_cast<int>(sortColumn)],
                sortColumn);
    }
}

void PlaylistTableModel::setTableModel(int playlistId) {
    //qDebug() << "PlaylistTableModel::setTableModel" << playlistId;
    if (m_iPlaylistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    m_iPlaylistId = playlistId;

    if (!m_keepDeletedTracks) {
        // From Mixxx 2.1 we drop tracks that have been explicitly deleted
        // in the library (mixxx_deleted = 0) from playlists.
        // These invisible tracks, consuming a playlist position number were
        // a source user of confusion in the past.
        m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().removeHiddenTracks(m_iPlaylistId);
    }

    QString playlistTableName = "playlist_" + QString::number(m_iPlaylistId);
    QSqlQuery query(m_database);
    FieldEscaper escaper(m_database);

    QStringList columns;
    columns << PLAYLISTTRACKSTABLE_TRACKID + " AS " + LIBRARYTABLE_ID
            << PLAYLISTTRACKSTABLE_POSITION
            << PLAYLISTTRACKSTABLE_DATETIMEADDED
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QString queryString = QString(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT %2 FROM PlaylistTracks "
            "INNER JOIN library ON library.id = PlaylistTracks.track_id "
            "WHERE PlaylistTracks.playlist_id = %3")
                                  .arg(escaper.escapeString(playlistTableName),
                                          columns.join(","),
                                          QString::number(playlistId));
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    // columns[1] = PLAYLISTTRACKSTABLE_POSITION from above
    // columns[2] = PLAYLISTTRACKSTABLE_DATETIMEADDED from above
    columns[3] = LIBRARYTABLE_PREVIEW;
    columns[4] = LIBRARYTABLE_COVERART;
    setTable(playlistTableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());
}

int PlaylistTableModel::addTracks(const QModelIndex& index,
        const QList<QString>& locations) {
    if (locations.isEmpty()) {
        return 0;
    }

    QList<TrackId> trackIds = m_pTrackCollectionManager->internalCollection()->resolveTrackIdsFromLocations(
            locations);

    const int positionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumn).data().toInt();

    // Handle weird cases like a drag and drop to an invalid index
    if (position <= 0) {
        position = rowCount() + 1;
    }

    int tracksAdded = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().insertTracksIntoPlaylist(
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
    return m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().appendTrackToPlaylist(trackId, m_iPlaylistId);
}

void PlaylistTableModel::removeTrack(const QModelIndex& index) {
    if (m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
    m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().removeTrackFromPlaylist(m_iPlaylistId, position);
}

void PlaylistTableModel::removeTracks(const QModelIndexList& indices) {
    if (m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId)) {
        return;
    }

    const int positionColumnIndex = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    QList<int> trackPositions;
    foreach (QModelIndex index, indices) {
        int trackPosition = index.sibling(index.row(), positionColumnIndex).data().toInt();
        trackPositions.append(trackPosition);
    }

    m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().removeTracksFromPlaylist(
            m_iPlaylistId,
            std::move(trackPositions));
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
        // Dragged out of bounds, which is past the end of the rows...
        newPosition = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().getMaxPosition(m_iPlaylistId);
    }

    m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().moveTrack(m_iPlaylistId, oldPosition, newPosition);
}

bool PlaylistTableModel::isLocked() {
    return m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
}

void PlaylistTableModel::shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude) {
    QList<int> positions;
    QHash<int, TrackId> allIds;
    const int positionColumn = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    const int idColumn = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID);
    int excludePos = -1;
    if (exclude.row() > -1) {
        // this is used to exclude the already loaded track at pos #1 if used from running Auto-DJ
        excludePos = exclude.sibling(exclude.row(), positionColumn).data().toInt();
    }
    if (shuffle.count() > 1) {
        // if there is more then one track selected, shuffle selection only
        foreach (QModelIndex shuffleIndex, shuffle) {
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
        TrackId trackId(index(i, idColumn).data());
        allIds.insert(position, trackId);
    }
    m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().shuffleTracks(m_iPlaylistId, positions, allIds);
}

bool PlaylistTableModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            TrackSetTableModel::isColumnInternal(column);
}

bool PlaylistTableModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED)) {
        return true;
    } else if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION)) {
        return false;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

TrackModel::Capabilities PlaylistTableModel::getCapabilities() const {
    TrackModel::Capabilities caps =
            Capability::ReceiveDrops |
            Capability::Reorder |
            Capability::AddToTrackSet |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::ResetPlayed;

    if (m_iPlaylistId !=
            m_pTrackCollectionManager->internalCollection()
                    ->getPlaylistDAO()
                    .getPlaylistIdFromName(AUTODJ_TABLE)) {
        // Only allow Add to AutoDJ if we aren't currently showing the AutoDJ queue.
        caps |= Capability::AddToAutoDJ | Capability::RemovePlaylist;
    } else {
        caps |= Capability::Remove;
    }
    if (PlaylistDAO::PLHT_SET_LOG ==
            m_pTrackCollectionManager->internalCollection()
                    ->getPlaylistDAO()
                    .getHiddenType(m_iPlaylistId)) {
        // Disable track reordering for history playlists
        caps &= ~(Capability::Reorder | Capability::RemovePlaylist);
    }
    bool locked = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO().isPlaylistLocked(m_iPlaylistId);
    if (locked) {
        caps |= Capability::Locked;
    }

    return caps;
}

void PlaylistTableModel::playlistsChanged(const QSet<int>& playlistIds) {
    if (playlistIds.contains(m_iPlaylistId)) {
        select(); // Repopulate the data model.
    }
}
