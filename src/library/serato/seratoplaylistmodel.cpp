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
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }

    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ARTIST] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TITLE] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUM] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUMARTIST] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_YEAR] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GENRE] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMPOSER] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GROUPING] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TRACKNUMBER] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILETYPE] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_NATIVELOCATION] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMMENT] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_DURATION] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BITRATE] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BPM] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_REPLAYGAIN] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_DATETIMEADDED] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TIMESPLAYED] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_RATING] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_KEY] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_PREVIEW] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COVERART] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_POSITION] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(m_columnIndexBySortColumnId[sortColumn], sortColumn);
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
