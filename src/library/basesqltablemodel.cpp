// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>
#include <QVector>
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
#include "util/time.h"
#include "util/dnd.h"

const bool sDebug = false;

BaseSqlTableModel::BaseSqlTableModel(QObject* pParent,
                                     TrackCollection* pTrackCollection,
                                     QString settingsNamespace)
        : QAbstractTableModel(pParent),
          TrackModel(pTrackCollection, settingsNamespace),
          m_pTrackCollection(pTrackCollection),
          m_previewDeckGroup(PlayerManager::groupForPreviewDeck(0)),
          m_iPreviewDeckTrackId(-1),
          m_pRowInfo(new QVector<RowInfo>),
          m_pNewRowInfo(new QVector<RowInfo>),
          m_currentSearch("") {
    m_bInitialized = false;
    m_iSortColumn = 0;
    m_eSortOrder = Qt::AscendingOrder;
    connect(&PlayerInfo::instance(), SIGNAL(trackLoaded(QString, TrackPointer)),
            this, SLOT(trackLoaded(QString, TrackPointer)));
    trackLoaded(m_previewDeckGroup, PlayerInfo::instance().getTrackInfo(m_previewDeckGroup));
}

BaseSqlTableModel::~BaseSqlTableModel() {
    delete m_pRowInfo;
    delete m_pNewRowInfo;
}

void BaseSqlTableModel::initHeaderData() {
    // Set the column heading labels, rename them for translations and have
    // proper capitalization
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED),
                  Qt::Horizontal, tr("Played"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST),
                  Qt::Horizontal, tr("Artist"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE),
                  Qt::Horizontal, tr("Title"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM),
                  Qt::Horizontal, tr("Album"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST),
                  Qt::Horizontal, tr("Album Artist"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE),
                  Qt::Horizontal, tr("Genre"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER),
                  Qt::Horizontal, tr("Composer"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING),
                  Qt::Horizontal, tr("Grouping"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR),
                  Qt::Horizontal, tr("Year"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE),
                  Qt::Horizontal, tr("Type"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION),
                  Qt::Horizontal, tr("Location"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT),
                  Qt::Horizontal, tr("Comment"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION),
                  Qt::Horizontal, tr("Duration"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING),
                  Qt::Horizontal, tr("Rating"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE),
                  Qt::Horizontal, tr("Bitrate"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER),
                  Qt::Horizontal, tr("Track #"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
                  Qt::Horizontal, tr("#"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Timestamp"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY),
                  Qt::Horizontal, tr("Key"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK),
                  Qt::Horizontal, tr("BPM Lock"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW),
                  Qt::Horizontal, tr("Preview"));
    setHeaderData(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART),
                  Qt::Horizontal, tr("Cover Art"));
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
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QString BaseSqlTableModel::orderByClause() const {
    bool tableColumnSort = m_iSortColumn < m_tableColumns.size();

    if (m_iSortColumn < 0 || !tableColumnSort) {
        return "";
    }

    QString field = m_idColumn;
    if (m_iSortColumn != 0) {
        field = m_tableColumns[m_iSortColumn];
    }

    QString s;
    s.append(QLatin1String("ORDER BY "));
    QString sort_field = QString("%1.%2").arg(m_tableName, field);
    s.append(sort_field);

#ifdef __SQLITE3__
    s.append(" COLLATE localeAwareCompare");
#endif
    s.append((m_eSortOrder == Qt::AscendingOrder) ? QLatin1String(" ASC") :
             QLatin1String(" DESC"));
    return s;
}

// must be called only from from TrackCollection thread (via callSync/callAsync)
// this is for sure because you need a trackcollectionPrivate for that
void BaseSqlTableModel::select(TrackCollectionPrivate* pTrackCollectionPrivate) {
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

    DBG() << "BaseSqlTableModel::select() start by" << this;
 //   Q_ASSERT(QThread::currentThread() != m_pTrackCollection->thread());

    QString columns = m_tableColumnsJoined;
    QString orderBy = orderByClause();
    QString queryString = QString("SELECT %1 FROM %2 %3")
                          .arg(columns, m_tableName, orderBy);

    QSqlQuery selectQuery(pTrackCollectionPrivate->getDatabase());
    // This causes a memory savings since QSqlCachedResult (what QtSQLite uses)
    // won't allocate a giant in-memory table that we won't use at all.
    selectQuery.setForwardOnly(true);
    selectQuery.prepare(queryString);

    if (!selectQuery.exec()) {
        LOG_FAILED_QUERY(selectQuery);
        return;
    }

    // Remove all the rows from the table. We wait to do this until after the
    // table query has succeeded. See Bug #1090888.
    // TODO(rryan) we could edit the table in place instead of clearing it?
    // TODO(MK) check if m_pRowInfo is correct here (due to merging)
    if (!m_pRowInfo->isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_pRowInfo->size() - 1);
        m_pRowInfo->clear();
        m_trackIdToRows.clear();
        endRemoveRows();
    }

    QSqlRecord record = selectQuery.record();
    int idColumn = record.indexOf(m_idColumn);

    QLinkedList<int> tableColumnIndices;
    foreach (QString column, m_tableColumns) {
        Q_ASSERT(record.indexOf(column) == m_tableColumnCache.fieldIndex(column));
        tableColumnIndices.push_back(record.indexOf(column));
    }

    // sqlite does not set size and m_rowInfo was just cleared
    //if (sDebug) {
    //    qDebug() << "Rows returned" << rows << m_rowInfo.size();
    //}

    if (m_pNewRowInfo->count()>0) {
        m_pNewRowInfo->clear();
    }

    QSet<int> trackIds;
    while (selectQuery.next()) {
        int id = selectQuery.value(idColumn).toInt();
        trackIds.insert(id);

        RowInfo thisRowInfo;
        thisRowInfo.trackId = id;
        thisRowInfo.order = m_pRowInfo->size(); // save rows where this currently track id is located
        // Get all the table columns and store them in the hash for this
        // row-info section.

        foreach (int tableColumnIndex, tableColumnIndices) {
            thisRowInfo.metadata[tableColumnIndex] =
                    selectQuery.value(tableColumnIndex);
        }
        m_pNewRowInfo->push_back(thisRowInfo);
    }

    if (sDebug) {
        qDebug() << "Rows actually received:" << m_pNewRowInfo->size();
    }

    // Adjust sort column to remove table columns and add 1 to add an id column.
    int sortColumn = m_iSortColumn - m_tableColumns.size() + 1;

    if (sortColumn < 0) {
        sortColumn = 0;
    }

    if (m_trackSource) {
        // If we were sorting a table column, then secondary sort by id. TODO(rryan)
        // we should look into being able to drop the secondary sort to save time
        // but going for correctness first.
        m_trackSource->filterAndSort(trackIds, m_currentSearch,
                                     m_currentSearchFilter,
                                     sortColumn, m_eSortOrder,
                                     &m_trackSortOrder,
                                     pTrackCollectionPrivate);

        // Re-sort the track IDs since filterAndSort can change their order or mark
        // them for removal (by setting their row to -1).
        for (QVector<RowInfo>::iterator it = m_pNewRowInfo->begin();
             it != m_pNewRowInfo->end(); ++it) {
            // If the sort column is not a track column then we will sort only to
            // separate removed tracks (order == -1) from present tracks (order ==
            // 0). Otherwise we sort by the order that filterAndSort returned to us.
            if (sortColumn == 0) {
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
    qStableSort(m_pNewRowInfo->begin(), m_pNewRowInfo->end());

    m_trackIdToRows.clear();
    for (int i = 0; i < m_pNewRowInfo->size(); ++i) {
        const RowInfo& row = m_pNewRowInfo->at(i);

        if (row.order == -1) {
            // We've reached the end of valid rows. Resize rowInfo to cut off
            // this and all further elements.
            m_pNewRowInfo->resize(i);
            break;
        }
        QLinkedList<int>& rows = m_trackIdToRows[row.trackId];
        rows.push_back(i);
    }

    // populating query results in UI thread
    MainExecuter::callAsync( [this] (){
        slotPopulateQueryResult();
    }, __PRETTY_FUNCTION__);
}

// Must be called from Main (GUI) Thread only
void BaseSqlTableModel::slotPopulateQueryResult() {
    DBG();
    Q_ASSERT(m_pRowInfo);
    // Remove all the rows from the table. We wait to do this until after the
    // table query has succeeded. See Bug #1090888.
    // TODO(rryan) we could edit the table in place instead of clearing it?
    if (m_pRowInfo->size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_pRowInfo->size()-1);
        m_pRowInfo->clear();
        m_trackIdToRows.clear();
        endRemoveRows();
    }

    // We're done! Issue the update signals and replace the master maps.
    // ToDo(MK): Check that m_pNewRowInfo is correct here (due to merging).
    //           Is it even ok to do the condition after reworking? Or should it be removed
    if (!m_pNewRowInfo->isEmpty()) {
        beginInsertRows(QModelIndex(), 0, m_pNewRowInfo->size()-1);

        QVector<RowInfo>* pOldRowInfo = m_pRowInfo;
        m_pRowInfo = m_pNewRowInfo;
        m_pNewRowInfo = pOldRowInfo;
        endInsertRows();
    }

    //int elapsed = time.elapsed();
    qDebug() << this << "select() finished"; //took" << elapsed << "ms" << rowInfo.size();
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
        disconnect(m_trackSource.data(), SIGNAL(tracksChanged(QSet<int>)),
                   this, SLOT(tracksChanged(QSet<int>)));
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
        connect(m_trackSource.data(), SIGNAL(tracksChanged(QSet<int>)),
                this, SLOT(tracksChanged(QSet<int>)), Qt::QueuedConnection);
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
    // here callAsync uses
    if (sDebug) {
        qDebug() << this << "search" << searchText;
    }
    setSearch(searchText, extraFilter);

    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
             [this] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        select(pTrackCollectionPrivate);
    }, __PRETTY_FUNCTION__);
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "setSort()" << column << order;
    }

    bool sortColumnChanged = m_iSortColumn != column;
    bool sortOrderChanged = m_eSortOrder != order;

    if (!sortColumnChanged && !sortOrderChanged) {
        // Do nothing if the sort is not different.
        return;
    }

    // TODO(rryan) optimization: if the sort column has not changed but the
    // order has, just reverse our ordering of the rows.

    m_iSortColumn = column;
    m_eSortOrder = order;
}

void BaseSqlTableModel::sort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "sort()" << column << order;
    }
    setSort(column, order);
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
             [this] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        select(pTrackCollectionPrivate);
    }, __PRETTY_FUNCTION__);
}

int BaseSqlTableModel::rowCount(const QModelIndex& parent) const {
    Q_ASSERT(m_pRowInfo!=NULL);
    int count = parent.isValid() ? 0 : m_pRowInfo->size();
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
                int year = value.toInt();
                if (year <= 0) {
                    // clear invalid values
                    value = QString();
                }
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
                // Otherwise, just use the column value.
            }

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

    if (row < 0 || row >= m_pRowInfo->size()) {
        return false;
    }

    const RowInfo& rowInfo = m_pRowInfo->at(row);
    int trackId = rowInfo.trackId;

    // You can't set something in the table columns because we have no way of
    // persisting it.
    const QHash<int, QVariant>& columns = rowInfo.metadata;
    if (columns.contains(column)) {
        return false;
    }

    // TODO(rryan) ugly and only works because the mixxx library tables are the
    // only ones that aren't read-only. This should be moved into BTC.
    TrackPointer pTrack;
		
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callSync(
             [trackId, &pTrack] (TrackCollectionPrivate* pTrackCollectionPrivate) {
				pTrack = pTrackCollectionPrivate->getTrackDAO().getTrack(trackId);
    }, __PRETTY_FUNCTION__);
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
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART)) {
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

const QLinkedList<int> BaseSqlTableModel::getTrackRows(int trackId) const {
    QHash<int, QLinkedList<int> >::const_iterator it =
            m_trackIdToRows.constFind(trackId);
    if (it != m_trackIdToRows.constEnd()) {
        return it.value();
    }
    return QLinkedList<int>();
}

int BaseSqlTableModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex(m_idColumn)).data().toInt();
}

// Must be called from Main thread
TrackPointer BaseSqlTableModel::getTrack(const QModelIndex& index) const {
    const int trackId = getTrackId(index);
    TrackPointer trackPointer;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
            [this, &trackId, &trackPointer] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        trackPointer = pTrackCollectionPrivate->getTrackDAO().getTrack(trackId);
    }, __PRETTY_FUNCTION__);
    return trackPointer;
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
        if (m_iPreviewDeckTrackId > -1) {
            const int numColumns = columnCount();
            QLinkedList<int> rows = getTrackRows(m_iPreviewDeckTrackId);
            foreach (int row, rows) {
                QModelIndex left = index(row, 0);
                QModelIndex right = index(row, numColumns);
                emit(dataChanged(left, right));
            }
        }
        m_iPreviewDeckTrackId = pTrack ? pTrack->getId() : -1;
    }
}

void BaseSqlTableModel::tracksChanged(QSet<int> trackIds) {
    if (sDebug) {
        qDebug() << this << "trackChanged" << trackIds.size();
    }

    const int numColumns = columnCount();
    foreach (int trackId, trackIds) {
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
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) == column) {
        pTrack->setType(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) == column) {
        pTrack->setTrackNumber(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LOCATION) == column) {
        pTrack->setLocation(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) == column) {
        pTrack->setComment(value.toString());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) == column) {
        pTrack->setDuration(value.toInt());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) == column) {
        pTrack->setBitrate(value.toInt());
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

    if (row < 0 || row >= m_pRowInfo->size()) {
        return QVariant();
    }

    // TODO(rryan) check range on column

    const RowInfo& rowInfo = m_pRowInfo->at(row);
    int trackId = rowInfo.trackId;

    // If the row info has the row-specific column, return that.
    const QHash<int, QVariant>& columns = rowInfo.metadata;
    if (columns.contains(column)) {
        // Special case for preview column. Return whether trackId is the
        // current preview deck track.
        if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
            if (role == Qt::ToolTipRole) {
                return "";
            }
            return m_iPreviewDeckTrackId == trackId;
        }

        if (sDebug) {
            qDebug() << "Returning table-column value" << columns[column]
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
        return new BPMDelegate(pParent, i, fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK));
    } else if (PlayerManager::numPreviewDecks() > 0 && i == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)) {
        return new PreviewButtonDelegate(pParent, i, m_pTrackCollection);
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

// Must be called from Main thread
void BaseSqlTableModel::hideTracks(const QModelIndexList& indices) {
    QList<int> trackIds;
    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this, trackIds] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        pTrackCollectionPrivate->getTrackDAO().hideTracks(trackIds);
        // TODO(rryan) : do not select, instead route event to BTC and notify from there.
        select(pTrackCollectionPrivate); //Repopulate the data model.
    }, __PRETTY_FUNCTION__);
}
