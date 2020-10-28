#include "library/serato/seratoplaylistmodel.h"

SeratoPlaylistModel::SeratoPlaylistModel(QObject* parent,
        TrackCollectionManager* trackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(
                  parent,
                  trackCollectionManager,
                  "mixxx.db.model.serato.playlistmodel",
                  "serato_playlists",
                  "serato_playlist_tracks",
                  trackSource) {
}

void SeratoPlaylistModel::initSortColumnMapping() {
    // Add a bijective mapping between the SortColumnIds and column indices
    for (int i = 0; i < static_cast<int>(TrackModel::SortColumnId::sortColumnIdMax); ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }

    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnArtist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnTitle)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnAlbum)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnAlbumArtist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnYear)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnGenre)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnComposer)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnGrouping)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnTrackNumber)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnFileType)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::sortColumnNativeLocation)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnComment)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnDuration)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnBitRate)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnBpm)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnReplayGain)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::sortColumnDateTimeAdded)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnTimesPlayed)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnRating)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnKey)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnPreview)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnCoverArt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::sortColumnPosition)] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = static_cast<int>(TrackModel::SortColumnId::sortColumnIdMin);
            i < static_cast<int>(TrackModel::SortColumnId::sortColumnIdMax);
            ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(
                m_columnIndexBySortColumnId[static_cast<int>(sortColumn)],
                sortColumn);
    }
}

TrackPointer SeratoPlaylistModel::getTrack(const QModelIndex& index) const {
    qDebug() << "SeratoTrackModel::getTrack";

    TrackPointer track = BaseExternalPlaylistModel::getTrack(index);

    return track;
}

bool SeratoPlaylistModel::isColumnHiddenByDefault(int column) {
    if (
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}
