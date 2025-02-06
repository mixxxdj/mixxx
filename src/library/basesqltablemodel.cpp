#include "library/basesqltablemodel.h"

#include <QUrl>
#include <QtDebug>
#include <algorithm>

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/starrating.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_basesqltablemodel.cpp"
#include "track/keyutils.h"
#include "track/track.h"
#include "track/trackmetadata.h"
#include "util/assert.h"
#include "util/datetime.h"
#include "util/db/dbconnection.h"
#include "util/duration.h"
#include "util/performancetimer.h"
#include "util/platform.h"

namespace {

const bool sDebug = false;

// The logic in the following code relies to a track column = 0
// Do not change it without changing the logic
// Column 0 is skipped when calculating the the columns of the view table
constexpr int kIdColumn = 0;
constexpr int kMaxSortColumns = 3;

// Constant for getModelSetting(name)
const QString COLUMNS_SORTING = QStringLiteral("ColumnsSorting");

const QString kModelName = "table:";

} // anonymous namespace

BaseSqlTableModel::BaseSqlTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace)
        : BaseTrackTableModel(parent, pTrackCollectionManager, settingsNamespace),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_database(pTrackCollectionManager->internalCollection()->database()),
          m_bInitialized(false) {
}

BaseSqlTableModel::~BaseSqlTableModel() {
}

void BaseSqlTableModel::initHeaderProperties() {
    BaseTrackTableModel::initHeaderProperties();
    // Add playlist columns
    setHeaderProperties(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION,
            tr("#"),
            30);
    setHeaderProperties(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED,
            tr("Timestamp"),
            80);
}

void BaseSqlTableModel::initSortColumnMapping() {
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
            TrackModel::SortColumnId::Color)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::CoverArt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::SampleRate)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::PlaylistDateTimeAdded)] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED);

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

void BaseSqlTableModel::clearRows() {
    DEBUG_ASSERT(m_rowInfo.empty() == m_trackIdToRows.empty());
    DEBUG_ASSERT(m_rowInfo.size() >= m_trackIdToRows.size());
    if (!m_rowInfo.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_rowInfo.size() - 1);
        m_rowInfo.clear();
        m_trackIdToRows.clear();
        m_trackPosToRow.clear();
        endRemoveRows();
    }
    DEBUG_ASSERT(m_rowInfo.isEmpty());
    DEBUG_ASSERT(m_trackIdToRows.isEmpty());
    DEBUG_ASSERT(m_trackPosToRow.isEmpty());
}

void BaseSqlTableModel::replaceRows(
        QVector<RowInfo>&& rows,
        TrackId2Rows&& trackIdToRows,
        TrackPos2Row&& trackPosToRows) {
    // NOTE(uklotzde): Use r-value references for parameters here, because
    // conceptually those parameters should replace the corresponding internal
    // member variables. Currently Qt4/5 doesn't support move semantics and
    // instead prevents unnecessary deep copying by implicit sharing (COW)
    // behind the scenes. Moving would be more efficient, although implicit
    // sharing meets all requirements. If Qt will ever add move support for
    // its container types in the future this code becomes even more efficient.
    DEBUG_ASSERT(rows.empty() == trackIdToRows.empty());
    DEBUG_ASSERT(rows.size() >= trackIdToRows.size());
    if (hasPositionColumn()) {
        DEBUG_ASSERT(rows.size() == trackPosToRows.size());
    }
    if (rows.isEmpty()) {
        clearRows();
    } else {
        beginInsertRows(QModelIndex(), 0, rows.size() - 1);
        m_rowInfo = rows;
        m_trackIdToRows = trackIdToRows;
        m_trackPosToRow = trackPosToRows;
        endInsertRows();
    }
}

void BaseSqlTableModel::select() {
    if (!m_bInitialized) {
        return;
    }
    // We should be able to detect when a select() would be a no-op. The DAO's
    // do not currently broadcast signals for when common things happen. In the
    // future, we can turn this check on and avoid a lot of needless
    // select()'s. rryan 9/2011
    // if (!m_bDirty) {
    //     if (sDebug) {
    //         qDebug() << this << "Skipping non-dirty select()";
    //     }
    //     return;
    // }

    if (sDebug) {
        qDebug() << this << "select()";
    }

    PerformanceTimer time;
    time.start();

    // Prepare query for id and all columns not in m_trackSource
    QString queryString = QString("SELECT %1 FROM %2 %3")
                                  .arg(m_tableColumns.join(","), m_tableName, m_tableOrderBy);

    if (sDebug) {
        qDebug() << this << "select() executing:" << queryString;
    }

    QSqlQuery query(m_database);
    // This causes a memory savings since QSqlCachedResult (what QtSQLite uses)
    // won't allocate a giant in-memory table that we won't use at all.
    query.setForwardOnly(true);
    if (!query.prepare(queryString)) {
        LOG_FAILED_QUERY(query);
        return;
    }
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Remove all the rows from the table after(!) the query has been
    // executed successfully. See issue #6782.
    // TODO(rryan) we could edit the table in place instead of clearing it?
    clearRows();

    // The size of the result set is not known in advance for a
    // forward-only query, so we cannot reserve memory for rows
    // in advance.
    QVector<RowInfo> rowInfos;
    QSet<TrackId> trackIds;
    int idColumn = -1;
    int posColumn = -1;
    while (query.next()) {
        QSqlRecord sqlRecord = query.record();

        if (idColumn < 0) {
            idColumn = sqlRecord.indexOf(m_idColumn);
        }

        if (posColumn == -1 && hasPositionColumn()) {
            posColumn = sqlRecord.indexOf(PLAYLISTTABLE_POSITION);
        }

        // TODO(XXX): Can we get rid of the hard-coded assumption that
        // the the first column always contains the id?
        DEBUG_ASSERT(idColumn == kIdColumn);

        VERIFY_OR_DEBUG_ASSERT(idColumn != -1) {
            qCritical()
                    << "ID column not available in database query results:"
                    << m_idColumn;
            return;
        }

        TrackId trackId(sqlRecord.value(idColumn));
        trackIds.insert(trackId);

        RowInfo rowInfo;
        rowInfo.trackId = trackId;
        rowInfo.row = rowInfos.size();

        rowInfo.columnValues.reserve(sqlRecord.count());
        for (int i = 0; i < m_tableColumns.size(); ++i) {
            rowInfo.columnValues.push_back(sqlRecord.value(i));
        }
        rowInfos.push_back(rowInfo);
    }

    if (sDebug) {
        qDebug() << "Rows actually received:" << rowInfos.size();
    }

    if (m_trackSource) {
        m_trackSource->filterAndSort(trackIds,
                m_currentSearch,
                m_currentSearchFilter,
                m_trackSourceOrderBy,
                m_sortColumns,
                m_tableColumns.size() - 1, // exclude the 1st column with the id
                &m_trackSortOrder);

        // Re-sort the track IDs since filterAndSort can change their order or mark
        // them for removal (by setting their row to -1).
        for (auto& rowInfo : rowInfos) {
            // If the sort is not a track column then we will sort only to
            // separate removed tracks (order == -1) from present tracks (order ==
            // 0). Otherwise we sort by the order that filterAndSort returned to us.
            if (m_trackSourceOrderBy.isEmpty()) {
                rowInfo.row = m_trackSortOrder.contains(rowInfo.trackId) ? 0 : -1;
            } else {
                rowInfo.row = m_trackSortOrder.value(rowInfo.trackId, -1);
            }
        }
    }

    // RowInfo::operator< sorts by the order field, except -1 is placed at the
    // end so we can easily slice off rows that are no longer present. Stable
    // sort is necessary because the tracks may be in pre-sorted order so we
    // should not disturb that if we are only removing tracks.
    std::stable_sort(rowInfos.begin(), rowInfos.end());

    TrackId2Rows trackIdToRows;
    // We expect almost all rows to be valid and that only a few tracks
    // are contained multiple times in rowInfos (e.g. in history playlists)
    trackIdToRows.reserve(rowInfos.size());
    for (int i = 0; i < rowInfos.size(); ++i) {
        const RowInfo& rowInfo = rowInfos[i];
        if (rowInfo.row == -1) {
            // We've reached the end of valid rows. Resize rowInfo to cut off
            // this and all further elements.
            rowInfos.resize(i);
            break;
        }
        trackIdToRows[rowInfo.trackId].push_back(i);
    }
    // The number of unique tracks cannot be greater than the
    // number of total rows returned by the query
    DEBUG_ASSERT(trackIdToRows.size() <= rowInfos.size());

    TrackPos2Row trackPosToRows;
    if (hasPositionColumn()) {
        // We expect as many positions as we have rows
        trackPosToRows.reserve(rowInfos.size());
        for (int i = 0; i < rowInfos.size(); ++i) {
            const RowInfo& rowInfo = rowInfos[i];
            trackPosToRows.insert(rowInfo.getPosition(posColumn), i);
        }
        DEBUG_ASSERT(trackPosToRows.size() == rowInfos.size());
    }

    // We're done! Issue the update signals and replace the main maps.
    replaceRows(
            std::move(rowInfos),
            std::move(trackIdToRows),
            std::move(trackPosToRows));
    // Both rowInfo and trackIdToRows (might) have been moved and
    // must not be used afterwards!

    qDebug() << this << "select() returned" << m_rowInfo.size()
             << "results in" << time.elapsed().debugMillisWithUnit();
}

void BaseSqlTableModel::setTable(QString tableName,
        QString idColumn,
        QStringList tableColumns,
        QSharedPointer<BaseTrackCache> trackSource) {
    if (sDebug) {
        qDebug() << this << "setTable" << tableName << tableColumns << idColumn;
    }
    m_tableName = std::move(tableName);
    m_idColumn = std::move(idColumn);
    m_tableColumns = std::move(tableColumns);

    if (m_trackSource) {
        disconnect(m_trackSource.data(),
                &BaseTrackCache::tracksChanged,
                this,
                &BaseSqlTableModel::tracksChanged);
    }
    m_trackSource = trackSource;
    if (m_trackSource) {
        // It's important that this not be a direct connection, or else the UI
        // might try to update while a cache operation is in progress, and that
        // will hit the cache again and cause dangerous reentry cycles
        // See https://github.com/mixxxdj/mixxx/issues/7569
        // TODO: A better fix is to have cache and trackpointers defer saving
        // and deleting, so those operations only take place at the top of
        // the call stack.
        connect(m_trackSource.data(),
                &BaseTrackCache::tracksChanged,
                this,
                &BaseSqlTableModel::tracksChanged,
                Qt::QueuedConnection);
    }

    initTableColumnsAndHeaderProperties(m_tableColumns);
    initSortColumnMapping();

    m_bInitialized = true;
}

int BaseSqlTableModel::columnIndexFromSortColumnId(TrackModel::SortColumnId column) const {
    if (column == TrackModel::SortColumnId::Invalid) {
        return -1;
    }

    return m_columnIndexBySortColumnId[static_cast<int>(column)];
}

TrackModel::SortColumnId BaseSqlTableModel::sortColumnIdFromColumnIndex(int index) const {
    return m_sortColumnIdByColumnIndex.value(index, TrackModel::SortColumnId::Invalid);
}

const QString BaseSqlTableModel::currentSearch() const {
    return m_currentSearch;
}

void BaseSqlTableModel::setSearch(const QString& searchText, const QString& extraFilter) {
    if (sDebug) {
        qDebug() << this << "setSearch" << searchText;
    }

    bool searchIsDifferent = m_currentSearch.isNull() || m_currentSearch != searchText;
    bool filterDisabled = (m_currentSearchFilter.isNull() && extraFilter.isNull());
    bool searchFilterIsDifferent = m_currentSearchFilter != extraFilter;

    if (!searchIsDifferent && (filterDisabled || !searchFilterIsDifferent)) {
        // Do nothing if the filters are no different.
        return;
    }

    m_currentSearch = searchText;
    m_currentSearchFilter = extraFilter;
}

void BaseSqlTableModel::search(const QString& searchText, const QString& extraFilter) {
    if (sDebug) {
        qDebug() << this << "search" << searchText;
    }
    setSearch(searchText, extraFilter);
    select();
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "setSort()" << column << order << m_tableColumns;
    }

    int trackSourceColumnCount = m_trackSource ? m_trackSource->columnCount() : 0;

    if (column < 0 ||
            column >= trackSourceColumnCount + m_tableColumns.size() - 1) {
        // -1 because id column is in both tables
        qWarning() << "BaseSqlTableModel::setSort invalid column:" << column;
        return;
    }

    // There's no item to sort already, load from Settings last sort
    if (m_sortColumns.isEmpty()) {
        QString val = getModelSetting(COLUMNS_SORTING);
        QTextStream in(&val);

        while (!in.atEnd()) {
            int ordI = -1;
            QString name;

            in >> name >> ordI;

            int col = fieldIndex(name);
            if (col < 0) {
                continue;
            }

            Qt::SortOrder ord;
            ord = ordI > 0 ? Qt::AscendingOrder : Qt::DescendingOrder;

            m_sortColumns << SortColumn(col, ord);
        }
    }
    if (m_sortColumns.size() > 0 && m_sortColumns.at(0).m_column == column) {
        // Only the order has changed
        m_sortColumns.replace(0, SortColumn(column, order));
    } else {
        // Remove column if already in history
        // As reverse loop to not skip an entry when removing the previous
        for (int i = m_sortColumns.size() - 1; i >= 0; --i) {
            if (m_sortColumns.at(i).m_column == column) {
                m_sortColumns.removeAt(i);
                break;
            }
        }

        // set new sort as head and shift out old sort
        m_sortColumns.prepend(SortColumn(column, order));

        if (m_sortColumns.size() > kMaxSortColumns) {
            m_sortColumns.removeLast();
        }
    }

    // Write new sortColumns order to user settings
    QString val;
    QTextStream out(&val);
    for (SortColumn& sc : m_sortColumns) {
        QString name;
        if (sc.m_column > 0 && sc.m_column < m_tableColumns.size()) {
            name = m_tableColumns[sc.m_column];
        } else {
            // ccColumn between 1..x to skip the id column
            int ccColumn = sc.m_column - m_tableColumns.size() + 1;
            name = m_trackSource->columnNameForFieldIndex(ccColumn);
        }

        out << name << " ";
        out << (sc.m_order == Qt::AscendingOrder ? 1 : -1) << " ";
    }
    out.flush();
    setModelSetting(COLUMNS_SORTING, val);

    if (sDebug) {
        qDebug() << "setSort() sortColumns:" << val;
    }

    // we have two selects for sorting, since keeping the select history
    // across the two selects is hard, we do this only for the trackSource
    // this is OK, because the columns of the table are virtual in case of
    // preview column or individual like playlist track number so that we
    // do not need the history anyway.

    // reset the old order by clauses
    m_trackSourceOrderBy.clear();
    m_tableOrderBy.clear();

    if (column > 0 && column < m_tableColumns.size()) {
        // Table sorting, no history
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
            // Random sort easter egg
            m_tableOrderBy = "ORDER BY RANDOM()";
        } else {
            m_tableOrderBy = "ORDER BY ";
            QString field = m_tableColumns[column];
            QString sort_field = QString("%1.%2").arg(m_tableName, field);
            m_tableOrderBy.append(mixxx::DbConnection::collateLexicographically(sort_field));
            m_tableOrderBy.append((order == Qt::AscendingOrder) ? " ASC" : " DESC");
        }
        m_sortColumns.clear();
        m_sortColumns.prepend(SortColumn(column, order));
    } else if (m_trackSource) {
        bool first = true;
        for (const SortColumn& sc : std::as_const(m_sortColumns)) {
            QString sort_field;
            if (sc.m_column < m_tableColumns.size()) {
                if (sc.m_column == kIdColumn) {
                    sort_field = m_trackSource->columnSortForFieldIndex(kIdColumn);
                } else if (sc.m_column ==
                        fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
                    sort_field = "RANDOM()";
                } else {
                    // we can't sort by other table columns here since primary sort is a track
                    // column: skip
                    continue;
                }
            } else {
                // + 1 to skip id column
                int ccColumn = sc.m_column - m_tableColumns.size() + 1;
                sort_field = m_trackSource->columnSortForFieldIndex(ccColumn);
            }
            VERIFY_OR_DEBUG_ASSERT(!sort_field.isEmpty()) {
                continue;
            }

            m_trackSourceOrderBy.append(first ? "ORDER BY " : ", ");
            m_trackSourceOrderBy.append(sort_field);
            m_trackSourceOrderBy.append((sc.m_order == Qt::AscendingOrder) ? " ASC" : " DESC");
            //qDebug() << m_trackSourceOrderBy;
            first = false;
        }
    }
}

void BaseSqlTableModel::sort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "sort()" << column << order;
    }
    setSort(column, order);
    select();
}

int BaseSqlTableModel::rowCount(const QModelIndex& parent) const {
    int count = parent.isValid() ? 0 : m_rowInfo.size();
    //qDebug() << "rowCount()" << parent << count;
    return count;
}

int BaseSqlTableModel::columnCount(const QModelIndex& parent) const {
    VERIFY_OR_DEBUG_ASSERT(!parent.isValid()) {
        return 0;
    }
    // Subtract one from trackSource::columnCount to ignore the id column
    int count = m_tableColumns.size() +
            (m_trackSource ? m_trackSource->columnCount() - 1 : 0);
    return count;
}

int BaseSqlTableModel::fieldIndex(ColumnCache::Column column) const {
    int tableIndex = BaseTrackTableModel::fieldIndex(column);
    if (tableIndex >= 0) {
        return tableIndex;
    }
    if (m_trackSource) {
        // We need to account for the case where the field name is not a table
        // column or a source column.
        int sourceTableIndex = m_trackSource->fieldIndex(column);
        if (sourceTableIndex > -1) {
            // Subtract one from the fieldIndex() result to account for the id column
            return m_tableColumns.size() + sourceTableIndex - 1;
        }
    }
    return tableIndex;
}

int BaseSqlTableModel::fieldIndex(const QString& fieldName) const {
    int tableIndex = BaseTrackTableModel::fieldIndex(fieldName);
    if (tableIndex >= 0) {
        return tableIndex;
    }
    if (m_trackSource) {
        // We need to account for the case where the field name is not a table
        // column or a source column.
        int sourceTableIndex = m_trackSource->fieldIndex(fieldName);
        if (sourceTableIndex > -1) {
            // Subtract one from the fieldIndex() result to account for the id column
            return m_tableColumns.size() + sourceTableIndex - 1;
        }
    }
    return tableIndex;
}

QString BaseSqlTableModel::modelKey(bool noSearch) const {
    if (noSearch) {
        return kModelName + m_tableName;
    }
    return kModelName + m_tableName +
            QStringLiteral("#") +
            currentSearch();
}

QVariant BaseSqlTableModel::rawValue(
        const QModelIndex& index) const {
    DEBUG_ASSERT(index.isValid());

    const int row = index.row();
    DEBUG_ASSERT(row >= 0);
    if (row >= m_rowInfo.size()) {
        return QVariant();
    }

    const int column = index.column();
    DEBUG_ASSERT(column >= 0);
    // TODO(rryan) check range on column

    const RowInfo& rowInfo = m_rowInfo[row];
    const TrackId trackId = rowInfo.trackId;

    // If the row info has the row-specific column, return that.
    if (column < m_tableColumns.size()) {
        // Special case for preview column. Return whether trackId is the
        // current preview deck track.
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
            return previewDeckTrackId() == trackId;
        }

        const QVector<QVariant>& columnValues = rowInfo.columnValues;
        if (sDebug) {
            qDebug() << "Returning table-column value"
                     << columnValues.at(column)
                     << "for column" << column;
        }
        return columnValues[column];
    }

    // Otherwise, return the information from the track record cache for the
    // given track ID
    if (!m_trackSource) {
        return QVariant();
    }
    // Subtract table columns from index to get the track source column
    // number and add 1 to skip over the id column.
    int trackSourceColumn = column - m_tableColumns.size() + 1;
    if (!m_trackSource->isCached(trackId)) {
        // Ideally Mixxx would have notified us of this via a signal, but in
        // the case that a track is not in the cache, we attempt to load it
        // on the fly. This will be a steep penalty to pay if there are tons
        // of these tracks in the table that are not cached.
        qDebug() << __FILE__ << __LINE__
                    << "Track" << trackId
                    << "was not present in cache and had to be manually fetched.";
        m_trackSource->ensureCached(trackId);
    }
    return m_trackSource->data(trackId, trackSourceColumn);
}

bool BaseSqlTableModel::setTrackValueForColumn(
        const TrackPointer& pTrack,
        int column,
        const QVariant& value,
        int role) {
    if (role != Qt::EditRole) {
        return false;
    }
    // You can't set something in the table columns because we have no way of
    // persisting it.
    if (column < m_tableColumns.size()) {
        return false;
    }

    // TODO(XXX) Qt properties could really help here.
    DEBUG_ASSERT(pTrack);
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST) == column) {
        pTrack->setArtist(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE) == column) {
        pTrack->setTitle(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM) == column) {
        pTrack->setAlbum(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST) == column) {
        pTrack->setAlbumArtist(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR) == column) {
        pTrack->setYear(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE) == column) {
        updateTrackGenre(pTrack.get(), value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER) == column) {
        pTrack->setComposer(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING) == column) {
        pTrack->setGrouping(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) == column) {
        pTrack->setTrackNumber(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) == column) {
        pTrack->setComment(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM) == column) {
        pTrack->trySetBpm(static_cast<double>(value.toDouble()));
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) == column) {
        // Update both the played flag and the number of times played
        pTrack->updatePlayCounter(value.toBool());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) == column) {
        const int timesPlayed = value.toInt();
        if (0 < timesPlayed) {
            // Preserve the played flag and only set the number of times played
            PlayCounter playCounter(pTrack->getPlayCounter());
            playCounter.setTimesPlayed(timesPlayed);
            pTrack->setPlayCounter(playCounter);
        } else {
            // Reset both the played flag and the number of times played
            pTrack->resetPlayCounter();
        }
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING) == column) {
        StarRating starRating = value.value<StarRating>();
        pTrack->setRating(starRating.starCount());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY) == column) {
        pTrack->setKeyText(
                value.toString(),
                mixxx::track::io::key::USER);
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) == column) {
        pTrack->setBpmLocked(value.toBool());
    } else {
        // We never should get up to this point!
        qWarning() << "Column"
                   << columnNameForFieldIndex(column)
                   << "is not editable!";
        DEBUG_ASSERT(false);
        return false;
    }
    return true;
}

TrackPointer BaseSqlTableModel::getTrack(const QModelIndex& index) const {
    return m_pTrackCollectionManager->getTrackById(getTrackId(index));
}

TrackId BaseSqlTableModel::getTrackId(const QModelIndex& index) const {
    if (index.isValid()) {
        return TrackId(index.sibling(index.row(), fieldIndex(m_idColumn)).data());
    } else {
        return TrackId();
    }
}

QString BaseSqlTableModel::getTrackLocation(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QString();
    }
    QString nativeLocation =
            index.sibling(index.row(),
                         fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION))
                    .data()
                    .toString();
    return QDir::fromNativeSeparators(nativeLocation);
}

QUrl BaseSqlTableModel::getTrackUrl(const QModelIndex& index) const {
    const QString trackLocation = getTrackLocation(index);
    DEBUG_ASSERT(trackLocation.trimmed() == trackLocation);
    if (trackLocation.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(trackLocation);
}

CoverInfo BaseSqlTableModel::getCoverInfo(const QModelIndex& index) const {
    CoverInfo coverInfo;
    coverInfo.setImageDigest(
            index.sibling(index.row(),
                         fieldIndex(ColumnCache::
                                         COLUMN_LIBRARYTABLE_COVERART_DIGEST))
                    .data()
                    .toByteArray(),
            index.sibling(index.row(),
                         fieldIndex(ColumnCache::
                                         COLUMN_LIBRARYTABLE_COVERART_HASH))
                    .data()
                    .toUInt());
    coverInfo.color = mixxx::RgbColor::fromQVariant(
            index.sibling(index.row(),
                         fieldIndex(ColumnCache::
                                         COLUMN_LIBRARYTABLE_COVERART_COLOR))
                    .data());
    if (coverInfo.hasCacheKey()) {
        coverInfo.type = static_cast<CoverInfo::Type>(
                index.sibling(index.row(),
                             fieldIndex(ColumnCache::
                                             COLUMN_LIBRARYTABLE_COVERART_TYPE))
                        .data()
                        .toInt());
        coverInfo.source = static_cast<CoverInfo::Source>(
                index.sibling(index.row(),
                             fieldIndex(ColumnCache::
                                             COLUMN_LIBRARYTABLE_COVERART_SOURCE))
                        .data()
                        .toInt());
        coverInfo.coverLocation =
                index.sibling(index.row(),
                             fieldIndex(ColumnCache::
                                             COLUMN_LIBRARYTABLE_COVERART_LOCATION))
                        .data()
                        .toString();
        coverInfo.trackLocation = getTrackLocation(index);
    }
    return coverInfo;
}

void BaseSqlTableModel::tracksChanged(const QSet<TrackId>& trackIds) {
    if (sDebug) {
        qDebug() << this << "trackChanged" << trackIds.size();
    }

    const int numColumns = columnCount();
    for (const auto& trackId : trackIds) {
        const auto rows = getTrackRows(trackId);
        for (int row : rows) {
            //qDebug() << "Row in this result set was updated. Signalling update. track:" << trackId << "row:" << row;
            QModelIndex topLeft = index(row, 0);
            QModelIndex bottomRight = index(row, numColumns);
            emit dataChanged(topLeft, bottomRight);
        }
    }
}

void BaseSqlTableModel::hideTracks(const QModelIndexList& indices) {
    QList<TrackId> trackIds;
    foreach (QModelIndex index, indices) {
        TrackId trackId(getTrackId(index));
        trackIds.append(trackId);
    }

    m_pTrackCollectionManager->hideTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}

QList<TrackRef> BaseSqlTableModel::getTrackRefs(
        const QModelIndexList& indices) const {
    QList<TrackRef> trackRefs;
    trackRefs.reserve(indices.size());
    foreach (QModelIndex index, indices) {
        trackRefs.append(TrackRef::fromFilePath(
                getTrackLocation(index),
                getTrackId(index)));
    }
    return trackRefs;
}
