// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>
#include <QUrl>

#include "library/basesqltablemodel.h"

#include "library/coverartdelegate.h"
#include "library/stardelegate.h"
#include "library/starrating.h"
#include "library/bpmdelegate.h"
#include "library/previewbuttondelegate.h"
#include "library/queryutil.h"
#include "playermanager.h"
#include "playerinfo.h"
#include "track/keyutils.h"
#include "metadata/trackmetadata.h"
#include "util/time.h"
#include "util/dnd.h"
#include "util/assert.h"

static const bool sDebug = false;

// The logic in the following code relies to a track column = 0
// Do not change it without changing the logic
// Column 0 is skipped when calculating the the columns of the view table
static const int kIdColumn = 0;
static const int kMaxSortColumns = 3;

BaseSqlTableModel::BaseSqlTableModel(QObject* pParent,
                                     TrackCollection* pTrackCollection,
                                     const char* settingsNamespace)
        : QAbstractTableModel(pParent),
          TrackModel(pTrackCollection->getDatabase(), settingsNamespace),
          m_pTrackCollection(pTrackCollection),
          m_trackDAO(pTrackCollection->getTrackDAO()),
          m_database(pTrackCollection->getDatabase()),
          m_previewDeckGroup(PlayerManager::groupForPreviewDeck(0)),
          m_bInitialized(false),
          m_currentSearch(""),
          m_trackSourceSortColumn(kIdColumn),
          m_trackSourceSortOrder(Qt::AscendingOrder) {
    connect(&PlayerInfo::instance(), SIGNAL(trackLoaded(QString, TrackPointer)),
            this, SLOT(trackLoaded(QString, TrackPointer)));
    connect(&m_trackDAO, SIGNAL(forceModelUpdate()),
            this, SLOT(select()));
    trackLoaded(m_previewDeckGroup, PlayerInfo::instance().getTrackInfo(m_previewDeckGroup));
}

BaseSqlTableModel::~BaseSqlTableModel() {
}

void BaseSqlTableModel::initHeaderData() {
    // Set the column heading labels, rename them for translations and have
    // proper capitalization

    // TODO(owilliams): Clean this up to make it readable.
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED,
                        tr("Played"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST,
                        tr("Artist"), 200);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_TITLE,
                        tr("Title"), 300);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM,
                        tr("Album"), 200);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST,
                        tr("Album Artist"), 100);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_GENRE,
                        tr("Genre"), 100);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER,
                        tr("Composer"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING,
                        tr("Grouping"), 10);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_YEAR,
                        tr("Year"), 40);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE,
                        tr("Type"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION,
                        tr("Location"), 100);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT,
                        tr("Comment"), 250);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_DURATION,
                        tr("Duration"), 70);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_RATING,
                        tr("Rating"), 100);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE,
                        tr("Bitrate"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_BPM,
                        tr("BPM"), 70);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER,
                        tr("Track #"), 10);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED,
                        tr("Date Added"), 90);
    setHeaderProperties(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION,
                        tr("#"), 30);
    setHeaderProperties(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED,
                        tr("Timestamp"), 80);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_KEY,
                        tr("Key"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK,
                        tr("BPM Lock"), 10);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW,
                        tr("Preview"), 50);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_COVERART,
                        tr("Cover Art"), 90);
    setHeaderProperties(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN,
                        tr("Replay Gain"), 50);
}

QSqlDatabase BaseSqlTableModel::database() const {
    return m_database;
}

void BaseSqlTableModel::setHeaderProperties(
        ColumnCache::Column column, QString title, int defaultWidth) {
    int fi = fieldIndex(column);
    setHeaderData(fi, Qt::Horizontal, m_tableColumnCache.columnName(column),
                  TrackModel::kHeaderNameRole);
    setHeaderData(fi, Qt::Horizontal, title, Qt::DisplayRole);
    setHeaderData(fi, Qt::Horizontal, defaultWidth, TrackModel::kHeaderWidthRole);
}

bool BaseSqlTableModel::setHeaderData(int section, Qt::Orientation orientation,
                                      const QVariant &value, int role) {
    int numColumns = columnCount();
    if (section < 0 || section >= numColumns) {
        return false;
    }

    if (orientation != Qt::Horizontal) {
        // We only care about horizontal headers.
        return false;
    }

    if (m_headerInfo.size() != numColumns) {
        m_headerInfo.resize(numColumns);
    }

    m_headerInfo[section][role] = value;
    emit(headerDataChanged(orientation, section, section));
    return true;
}

QVariant BaseSqlTableModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        QVariant headerValue = m_headerInfo.value(section).value(role);
        if (!headerValue.isValid()) {
            // Try EditRole if DisplayRole wasn't present
            headerValue = m_headerInfo.value(section).value(Qt::EditRole);
        }
        if (!headerValue.isValid()) {
            headerValue = QVariant(section).toString();
        }
        return headerValue;
    } else if (role == TrackModel::kHeaderWidthRole && orientation == Qt::Horizontal) {
        QVariant widthValue = m_headerInfo.value(section).value(role);
        if (!widthValue.isValid()) {
            return 50;
        }
        return widthValue;
    } else if (role == TrackModel::kHeaderNameRole && orientation == Qt::Horizontal) {
        return m_headerInfo.value(section).value(role);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


bool BaseSqlTableModel::isColumnHiddenByDefault(int column) {
    if ((column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST)) ||
            (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN))) {
        return true;
    }
    return false;
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

    QTime time;
    time.start();

    // Prepare query for id and all columns not in m_trackSource
    QString queryString = QString("SELECT %1 FROM %2 %3")
            .arg(m_tableColumnsJoined, m_tableName, m_tableOrderBy);

    if (sDebug) {
        qDebug() << this << "select() executing:" << queryString;
    }

    QSqlQuery query(m_database);
    // This causes a memory savings since QSqlCachedResult (what QtSQLite uses)
    // won't allocate a giant in-memory table that we won't use at all.
    query.setForwardOnly(true);
    query.prepare(queryString);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Remove all the rows from the table. We wait to do this until after the
    // table query has succeeded. See Bug #1090888.
    // TODO(rryan) we could edit the table in place instead of clearing it?
    if (!m_rowInfo.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_rowInfo.size() - 1);
        m_rowInfo.clear();
        m_trackIdToRows.clear();
        endRemoveRows();
    }
    // sqlite does not set size and m_rowInfo was just cleared
    //if (sDebug) {
    //    qDebug() << "Rows returned" << rows << m_rowInfo.size();
    //}

    QVector<RowInfo> rowInfo;
    QSet<TrackId> trackIds;
    while (query.next()) {
        TrackId trackId(query.value(kIdColumn));
        trackIds.insert(trackId);

        RowInfo thisRowInfo;
        thisRowInfo.trackId = trackId;
        // save rows where this currently track id is located
        thisRowInfo.order = rowInfo.size();
        // Get all the table columns and store them in the hash for this
        // row-info section.

        thisRowInfo.metadata.reserve(m_tableColumns.size());
        for (int i = 0;  i < m_tableColumns.size(); ++i) {
            thisRowInfo.metadata << query.value(i);
        }
        rowInfo.push_back(thisRowInfo);
    }

    if (sDebug) {
        qDebug() << "Rows actually received:" << rowInfo.size();
    }

    if (m_trackSource) {
        m_trackSource->filterAndSort(trackIds, m_currentSearch,
                                     m_currentSearchFilter,
                                     m_trackSourceOrderBy,
                                     m_trackSourceSortColumn,
                                     m_trackSourceSortOrder,
                                     &m_trackSortOrder);

        // Re-sort the track IDs since filterAndSort can change their order or mark
        // them for removal (by setting their row to -1).
        for (QVector<RowInfo>::iterator it = rowInfo.begin();
                it != rowInfo.end(); ++it) {
            // If the sort is not a track column then we will sort only to
            // separate removed tracks (order == -1) from present tracks (order ==
            // 0). Otherwise we sort by the order that filterAndSort returned to us.
            if (m_trackSourceOrderBy.isEmpty()) {
                it->order = m_trackSortOrder.contains(it->trackId) ? 0 : -1;
            } else {
                it->order = m_trackSortOrder.value(it->trackId, -1);
            }
        }
    }

    // RowInfo::operator< sorts by the order field, except -1 is placed at the
    // end so we can easily slice off rows that are no longer present. Stable
    // sort is necessary because the tracks may be in pre-sorted order so we
    // should not disturb that if we are only removing tracks.
    qStableSort(rowInfo.begin(), rowInfo.end());

    m_trackIdToRows.clear();
    for (int i = 0; i < rowInfo.size(); ++i) {
        const RowInfo& row = rowInfo[i];

        if (row.order == -1) {
            // We've reached the end of valid rows. Resize rowInfo to cut off
            // this and all further elements.
            rowInfo.resize(i);
            break;
        }
        QLinkedList<int>& rows = m_trackIdToRows[row.trackId];
        rows.push_back(i);
    }

    // We're done! Issue the update signals and replace the master maps.
    if (!rowInfo.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, rowInfo.size() - 1);
        m_rowInfo = rowInfo;
        endInsertRows();
    }

    int elapsed = time.elapsed();
    qDebug() << this << "select() took" << elapsed << "ms" << rowInfo.size();
}

void BaseSqlTableModel::setTable(const QString& tableName,
                                 const QString& idColumn,
                                 const QStringList& tableColumns,
                                 QSharedPointer<BaseTrackCache> trackSource) {
    if (sDebug) {
        qDebug() << this << "setTable" << tableName << tableColumns << idColumn;
    }
    m_tableName = tableName;
    m_idColumn = idColumn;
    m_tableColumns = tableColumns;
    m_tableColumnsJoined = tableColumns.join(",");

    if (m_trackSource) {
        disconnect(m_trackSource.data(), SIGNAL(tracksChanged(QSet<TrackId>)),
                   this, SLOT(tracksChanged(QSet<TrackId>)));
    }
    m_trackSource = trackSource;
    if (m_trackSource) {
        // It's important that this not be a direct connection, or else the UI
        // might try to update while a cache operation is in progress, and that
        // will hit the cache again and cause dangerous reentry cycles
        // See https://bugs.launchpad.net/mixxx/+bug/1365708
        // TODO: A better fix is to have cache and trackpointers defer saving
        // and deleting, so those operations only take place at the top of
        // the call stack.
        connect(m_trackSource.data(), SIGNAL(tracksChanged(QSet<TrackId>)),
                this, SLOT(tracksChanged(QSet<TrackId>)), Qt::QueuedConnection);
    }

    // Build a map from the column names to their indices, used by fieldIndex()
    m_tableColumnCache.setColumns(m_tableColumns);

    initHeaderData();

    m_bInitialized = true;
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
        qDebug() << this << "setSort()" << column << order;
    }

    int trackSourceColumnCount = m_trackSource ? m_trackSource->columnCount() : 0;

    if (column < 0 ||
            column >= trackSourceColumnCount + m_sortColumns.size() - 1) {
        // -1 because id column is in both tables
        qWarning() << "BaseSqlTableModel::setSort invalid column:" << column;
        return;
    }

    if (m_sortColumns.size() > 0 &&
            m_sortColumns.at(0).m_column == column) {
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

    // we have two selects for sorting, since keeping the select history
    // across the two selects is hard, we do this only for the trackSource
    // this is OK, because the colums of the table are virtual in case of
    // preview column or individual like playlist track number so that we
    // do not need the history anyway.

    // reset the old order by clauses
    m_trackSourceOrderBy.clear();
    m_tableOrderBy.clear();
    m_trackSourceSortColumn = 0;
    m_trackSourceSortOrder = Qt::AscendingOrder;

    if (column > 0 && column < m_tableColumns.size()) {
        // Table sorting, no history
        m_tableOrderBy.append("ORDER BY ");
        QString field = m_tableColumns[column];
        QString sort_field = QString("%1.%2").arg(m_tableName, field);
        m_tableOrderBy.append(sort_field);
    #ifdef __SQLITE3__
        m_tableOrderBy.append(" COLLATE localeAwareCompare");
    #endif
        m_tableOrderBy.append((order == Qt::AscendingOrder) ?
                " ASC" : " DESC");
        m_sortColumns.clear();
    } else if (m_trackSource) {
        for (int i = 0; i < m_sortColumns.size(); ++i) {
            SortColumn sc = m_sortColumns.at(i);
            // TrackSource Sorting, current sort + two from history
            if (i == 0) {
                m_trackSourceOrderBy.append("ORDER BY ");
            } else {
                // second cycle
                m_trackSourceOrderBy.append(", ");
            }
            QString sort_field;
            if (sc.m_column == kIdColumn) {
                sort_field = m_trackSource->columnSortForFieldIndex(kIdColumn);
            } else {
                // + 1 to skip id column
                int ccColumn = sc.m_column - m_tableColumns.size() + 1;
                sort_field = m_trackSource->columnSortForFieldIndex(ccColumn);
                if (i == 0) {
                    // first cycle: main sort criteria
                    m_trackSourceSortColumn = ccColumn;
                    m_trackSourceSortOrder = sc.m_order;
                }
            }

            m_trackSourceOrderBy.append(sort_field);

    #ifdef __SQLITE3__
            m_trackSourceOrderBy.append(" COLLATE localeAwareCompare");
    #endif
            m_trackSourceOrderBy.append((sc.m_order == Qt::AscendingOrder) ?
                    " ASC" : " DESC");
            //qDebug() << m_trackSourceOrderBy;
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
    if (parent.isValid()) {
        return 0;
    }

    // Subtract one from trackSource::columnCount to ignore the id column
    int count = m_tableColumns.size() +
                (m_trackSource ? m_trackSource->columnCount() - 1: 0);
    return count;
}

int BaseSqlTableModel::fieldIndex(ColumnCache::Column column) const {
    int tableIndex = m_tableColumnCache.fieldIndex(column);
    if (tableIndex > -1) {
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
    return -1;
}

int BaseSqlTableModel::fieldIndex(const QString& fieldName) const {
    int tableIndex = m_tableColumnCache.fieldIndex(fieldName);
    if (tableIndex > -1) {
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
    return -1;
}

QVariant BaseSqlTableModel::data(const QModelIndex& index, int role) const {
    //qDebug() << this << "data()";
    if (!index.isValid() || (role != Qt::DisplayRole &&
                             role != Qt::EditRole &&
                             role != Qt::CheckStateRole &&
                             role != Qt::ToolTipRole)) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    // This value is the value in its most raw form. It was looked up either
    // from the SQL table or from the cached track layer.
    QVariant value = getBaseValue(index, role);

    // Format the value based on whether we are in a tooltip, display, or edit
    // role
    switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
            if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION)) {
                int duration = value.toInt();
                if (duration > 0) {
                    value = Time::formatSeconds(duration, false);
                } else {
                    value = QString();
                }
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
                if (qVariantCanConvert<int>(value))
                    value = qVariantFromValue(StarRating(value.toInt()));
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
                if (qVariantCanConvert<int>(value))
                    value =  QString("(%1)").arg(value.toInt());
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)) {
                value = value.toBool();
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED)) {
                QDateTime gmtDate = value.toDateTime();
                gmtDate.setTimeSpec(Qt::UTC);
                value = gmtDate.toLocalTime();
            } else if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED)) {
                QDateTime gmtDate = value.toDateTime();
                gmtDate.setTimeSpec(Qt::UTC);
                value = gmtDate.toLocalTime();
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) {
                value = value.toBool();
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR)) {
                value = Mixxx::TrackMetadata::formatCalendarYear(value.toString());
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER)) {
                int track_number = value.toInt();
                if (track_number <= 0) {
                    // clear invalid values
                    value = QString();
                }
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE)) {
                int bitrate = value.toInt();
                if (bitrate <= 0) {
                    // clear invalid values
                    value = QString();
                }
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY)) {
                // If we know the semantic key via the LIBRARYTABLE_KEY_ID
                // column (as opposed to the string representation of the key
                // currently stored in the DB) then lookup the key and render it
                // using the user's selected notation.
                int keyIdColumn = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID);
                if (keyIdColumn != -1) {
                    mixxx::track::io::key::ChromaticKey key =
                            KeyUtils::keyFromNumericValue(
                                index.sibling(row, keyIdColumn).data().toInt());

                    if (key != mixxx::track::io::key::INVALID) {
                        // Render this key with the user-provided notation.
                        value = KeyUtils::keyToString(key);
                    }
                }
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN)) {
                value = Mixxx::ReplayGain::ratioToString(value.toDouble());
            } // Otherwise, just use the column value.

            break;
        case Qt::EditRole:
            if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
                value = value.toDouble();
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
                value = index.sibling(
                    row, fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)).data().toBool();
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
                if (qVariantCanConvert<int>(value)) {
                    value = qVariantFromValue(StarRating(value.toInt()));
                }
            }
            break;
        case Qt::CheckStateRole:
            if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
                bool played = index.sibling(
                        row, fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED)).data().toBool();
                value = played ? Qt::Checked : Qt::Unchecked;
            } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
                bool locked = index.sibling(
                        row, fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)).data().toBool();
                value = locked ? Qt::Checked : Qt::Unchecked;
            }
            break;
        default:
            break;
    }
    return value;
}

bool BaseSqlTableModel::setData(
    const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;

    int row = index.row();
    int column = index.column();

    if (sDebug) {
        qDebug() << this << "setData() column:" << column << "value:" << value << "role:" << role;
    }

    // Over-ride sets to TIMESPLAYED and re-direct them to PLAYED
    if (role == Qt::CheckStateRole) {
        QString val = value.toInt() > 0 ? QString("true") : QString("false");
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED)) {
            QModelIndex playedIndex = index.sibling(index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED));
            return setData(playedIndex, val, Qt::EditRole);
        } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
            QModelIndex bpmLockindex = index.sibling(index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK));
            return setData(bpmLockindex, val, Qt::EditRole);
        }
        return false;
    }

    if (row < 0 || row >= m_rowInfo.size()) {
        return false;
    }

    const RowInfo& rowInfo = m_rowInfo[row];
    TrackId trackId(rowInfo.trackId);

    // You can't set something in the table columns because we have no way of
    // persisting it.
    if (column < m_tableColumns.size()) {
        return false;
    }

    // TODO(rryan) ugly and only works because the mixxx library tables are the
    // only ones that aren't read-only. This should be moved into BTC.
    TrackPointer pTrack = m_trackDAO.getTrack(trackId);
    if (!pTrack) {
        return false;
    }
    setTrackValueForColumn(pTrack, column, value);

    // Do not save the track here. Changing the track dirties it and the caching
    // system will automatically save the track once it is unloaded from
    // memory. rryan 10/2010
    //m_trackDAO.saveTrack(pTrack);

    return true;
}

Qt::ItemFlags BaseSqlTableModel::flags(const QModelIndex &index) const {
    return readWriteFlags(index);
}

Qt::ItemFlags BaseSqlTableModel::readWriteFlags(
    const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    // Enable dragging songs from this data model to elsewhere (like the
    // waveform widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    int column = index.column();

    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN)) {
        return defaultFlags;
    } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED))  {
        return defaultFlags | Qt::ItemIsUserCheckable;
    } else if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK)) {
        return defaultFlags | Qt::ItemIsUserCheckable;
    } else if(column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
        // Allow checking of the BPM-locked indicator.
        defaultFlags |= Qt::ItemIsUserCheckable;
        // Disable editing of BPM field when BPM is locked
        bool locked = index.sibling(
            index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK))
                .data().toBool();
        return locked ? defaultFlags : defaultFlags | Qt::ItemIsEditable;
    } else {
        return defaultFlags | Qt::ItemIsEditable;
    }
}

Qt::ItemFlags BaseSqlTableModel::readOnlyFlags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    // Enable dragging songs from this data model to elsewhere (like the
    // waveform widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

const QLinkedList<int> BaseSqlTableModel::getTrackRows(TrackId trackId) const {
    QHash<TrackId, QLinkedList<int> >::const_iterator it =
            m_trackIdToRows.constFind(trackId);
    if (it != m_trackIdToRows.constEnd()) {
        return it.value();
    }
    return QLinkedList<int>();
}

TrackId BaseSqlTableModel::getTrackId(const QModelIndex& index) const {
    if (index.isValid()) {
        return TrackId(index.sibling(index.row(), fieldIndex(m_idColumn)).data());
    } else {
        return TrackId();
    }
}

TrackPointer BaseSqlTableModel::getTrack(const QModelIndex& index) const {
    return m_trackDAO.getTrack(getTrackId(index));
}

QString BaseSqlTableModel::getTrackLocation(const QModelIndex& index) const {
    if (!index.isValid()) {
        return "";
    }
    QString location = index.sibling(
        index.row(), fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION)).data().toString();
    return location;
}

void BaseSqlTableModel::trackLoaded(QString group, TrackPointer pTrack) {
    if (group == m_previewDeckGroup) {
        // If there was a previously loaded track, refresh its rows so the
        // preview state will update.
        if (m_previewDeckTrackId.isValid()) {
            const int numColumns = columnCount();
            QLinkedList<int> rows = getTrackRows(m_previewDeckTrackId);
            m_previewDeckTrackId = TrackId(); // invalidate
            foreach (int row, rows) {
                QModelIndex left = index(row, 0);
                QModelIndex right = index(row, numColumns);
                emit(dataChanged(left, right));
            }
        }
        m_previewDeckTrackId = pTrack ? pTrack->getId() : TrackId();
    }
}

void BaseSqlTableModel::tracksChanged(QSet<TrackId> trackIds) {
    if (sDebug) {
        qDebug() << this << "trackChanged" << trackIds.size();
    }

    const int numColumns = columnCount();
    for (const auto& trackId : trackIds) {
        QLinkedList<int> rows = getTrackRows(trackId);
        foreach (int row, rows) {
            //qDebug() << "Row in this result set was updated. Signalling update. track:" << trackId << "row:" << row;
            QModelIndex left = index(row, 0);
            QModelIndex right = index(row, numColumns);
            emit(dataChanged(left, right));
        }
    }
}

void BaseSqlTableModel::setTrackValueForColumn(TrackPointer pTrack, int column,
                                               QVariant value) {
    // TODO(XXX) Qt properties could really help here.
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
        pTrack->setGenre(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER) == column) {
        pTrack->setComposer(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING) == column) {
        pTrack->setGrouping(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) == column) {
        pTrack->setTrackNumber(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) == column) {
        pTrack->setComment(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM) == column) {
        // QVariant::toFloat needs >= QT 4.6.x
        pTrack->setBpm(static_cast<double>(value.toDouble()));
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) == column) {
        pTrack->setPlayedAndUpdatePlaycount(value.toBool());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) == column) {
        pTrack->setTimesPlayed(value.toInt());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING) == column) {
        StarRating starRating = qVariantValue<StarRating>(value);
        pTrack->setRating(starRating.starCount());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY) == column) {
        pTrack->setKeyText(value.toString(),
                           mixxx::track::io::key::USER);
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) == column) {
        pTrack->setBpmLock(value.toBool());
    } else {
        // We never should get up to this point!
        DEBUG_ASSERT_AND_HANDLE(false) {
            qWarning() << "Column"
                    << m_tableColumnCache.columnNameForFieldIndex(column)
                    << "is not editable!";
        }
    }
}

QVariant BaseSqlTableModel::getBaseValue(
    const QModelIndex& index, int role) const {
    if (role != Qt::DisplayRole &&
        role != Qt::ToolTipRole &&
        role != Qt::EditRole) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_rowInfo.size()) {
        return QVariant();
    }

    // TODO(rryan) check range on column

    const RowInfo& rowInfo = m_rowInfo[row];
    TrackId trackId(rowInfo.trackId);

    // If the row info has the row-specific column, return that.
    if (column < m_tableColumns.size()) {
        // Special case for preview column. Return whether trackId is the
        // current preview deck track.
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
            if (role == Qt::ToolTipRole) {
                return "";
            }
            return m_previewDeckTrackId == trackId;
        }

        const QVector<QVariant>& columns = rowInfo.metadata;
        if (sDebug) {
            qDebug() << "Returning table-column value" << columns.at(column)
                     << "for column" << column << "role" << role;
        }
        return columns[column];
    }

    // Otherwise, return the information from the track record cache for the
    // given track ID
    if (m_trackSource) {
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
    return QVariant();
}

QMimeData* BaseSqlTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    // The list of indexes we're given contains separates indexes for each
    // column, so even if only one row is selected, we'll have columnCount()
    // indices.  We need to only count each row once:
    QSet<int> rows;

    foreach (QModelIndex index, indexes) {
        if (!index.isValid() || rows.contains(index.row())) {
            continue;
        }
        rows.insert(index.row());
        QUrl url = DragAndDropHelper::urlFromLocation(getTrackLocation(index));
        if (!url.isValid()) {
            qDebug() << this << "ERROR: invalid url" << url;
            continue;
        }
        urls.append(url);
    }
    mimeData->setUrls(urls);
    return mimeData;
}

QAbstractItemDelegate* BaseSqlTableModel::delegateForColumn(const int i, QObject* pParent) {
    if (i == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING)) {
        return new StarDelegate(pParent);
    } else if (i == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM)) {
        return new BPMDelegate(pParent);
    } else if (PlayerManager::numPreviewDecks() > 0 && i == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
        return new PreviewButtonDelegate(pParent, i);
    } else if (i == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART)) {
        CoverArtDelegate* pCoverDelegate = new CoverArtDelegate(pParent);
        connect(pCoverDelegate, SIGNAL(coverReadyForCell(int, int)),
                this, SLOT(refreshCell(int, int)));
        return pCoverDelegate;
    }
    return NULL;
}

void BaseSqlTableModel::refreshCell(int row, int column) {
    QModelIndex coverIndex = index(row, column);
    emit(dataChanged(coverIndex, coverIndex));
}

void BaseSqlTableModel::hideTracks(const QModelIndexList& indices) {
    QList<TrackId> trackIds;
    foreach (QModelIndex index, indices) {
        TrackId trackId(getTrackId(index));
        trackIds.append(trackId);
    }

    m_trackDAO.hideTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}
