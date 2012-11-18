// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>

#include "library/basesqltablemodel.h"

#include "library/starrating.h"
#include "library/stardelegate.h"
#include "mixxxutils.cpp"

const bool sDebug = false;

BaseSqlTableModel::BaseSqlTableModel(QObject* pParent,
                                     TrackCollection* pTrackCollection,
                                     QSqlDatabase db,
                                     QString settingsNamespace)
        :  QAbstractTableModel(pParent),
           TrackModel(db, settingsNamespace),
           m_currentSearch(""),
           m_pTrackCollection(pTrackCollection),
           m_trackDAO(m_pTrackCollection->getTrackDAO()),
           m_database(db) {
    m_bInitialized = false;
    m_bDirty = true;
    m_iSortColumn = 0;
    m_eSortOrder = Qt::AscendingOrder;
}

BaseSqlTableModel::~BaseSqlTableModel() {
}

void BaseSqlTableModel::initHeaderData() {
    // Set the column heading labels, rename them for translations and have
    // proper capitalization
    setHeaderData(fieldIndex(LIBRARYTABLE_TIMESPLAYED),
                  Qt::Horizontal, tr("Played"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ARTIST),
                  Qt::Horizontal, tr("Artist"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TITLE),
                  Qt::Horizontal, tr("Title"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ALBUM),
                  Qt::Horizontal, tr("Album"));
    setHeaderData(fieldIndex(LIBRARYTABLE_GENRE),
                  Qt::Horizontal, tr("Genre"));
    setHeaderData(fieldIndex(LIBRARYTABLE_COMPOSER),
                  Qt::Horizontal, tr("Composer"));
    setHeaderData(fieldIndex(LIBRARYTABLE_YEAR),
                  Qt::Horizontal, tr("Year"));
    setHeaderData(fieldIndex(LIBRARYTABLE_FILETYPE),
                  Qt::Horizontal, tr("Type"));
    setHeaderData(fieldIndex(LIBRARYTABLE_LOCATION),
                  Qt::Horizontal, tr("Location"));
    setHeaderData(fieldIndex(LIBRARYTABLE_COMMENT),
                  Qt::Horizontal, tr("Comment"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DURATION),
                  Qt::Horizontal, tr("Duration"));
    setHeaderData(fieldIndex(LIBRARYTABLE_RATING),
                  Qt::Horizontal, tr("Rating"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BITRATE),
                  Qt::Horizontal, tr("Bitrate"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TRACKNUMBER),
                  Qt::Horizontal, tr("Track #"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));
    setHeaderData(fieldIndex(PLAYLISTTRACKSTABLE_POSITION),
                  Qt::Horizontal, tr("#"));
    setHeaderData(fieldIndex(PLAYLISTTRACKSTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Timestamp"));
    setHeaderData(fieldIndex(LIBRARYTABLE_KEY),
                  Qt::Horizontal, tr("Key"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM_LOCK),
                  Qt::Horizontal, tr("BPM Lock"));
}

QSqlDatabase BaseSqlTableModel::database() const {
    return m_database;
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
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Horizontal) {
        QVariant headerValue = m_headerInfo.value(section).value(role);
        if (!headerValue.isValid() && role == Qt::DisplayRole) {
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

    s.append((m_eSortOrder == Qt::AscendingOrder) ? QLatin1String(" ASC") :
             QLatin1String(" DESC"));
    return s;
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

    // Remove all the rows from the table.
    // TODO(rryan) we could edit the table in place instead of clearing it?
    if (m_rowInfo.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_rowInfo.size()-1);
        m_rowInfo.clear();
        m_trackIdToRows.clear();
        endRemoveRows();
    }

    QString columns = m_tableColumnsJoined;
    QString orderBy = orderByClause();
    QString queryString = QString("SELECT %1 FROM %2 %3")
            .arg(columns, m_tableName, orderBy);

    if (sDebug) {
        qDebug() << this << "select() executing:" << queryString;
    }

    QSqlQuery query(m_database);
    // This causes a memory savings since QSqlCachedResult (what QtSQLite uses)
    // won't allocate a giant in-memory table that we won't use at all.
    query.setForwardOnly(true);
    query.prepare(queryString);

    if (!query.exec()) {
        qDebug() << this << "select() error:" << __FILE__ << __LINE__
                 << query.executedQuery() << query.lastError();
    }

    QSqlRecord record = query.record();
    int idColumn = record.indexOf(m_idColumn);

    QLinkedList<int> tableColumnIndices;
    foreach (QString column, m_tableColumns) {
        Q_ASSERT(record.indexOf(column) == m_tableColumnIndex[column]);
        tableColumnIndices.push_back(record.indexOf(column));
    }

	// sqlite does not set size and m_rowInfo was just cleared    
    //int rows = query.size();
    //if (sDebug) {
    //    qDebug() << "Rows returned" << rows << m_rowInfo.size();
    //}

    QVector<RowInfo> rowInfo;
    QSet<int> trackIds;
    while (query.next()) {
        int id = query.value(idColumn).toInt();
        trackIds.insert(id);

        RowInfo thisRowInfo;
        thisRowInfo.trackId = id;
        thisRowInfo.order = rowInfo.size(); // save rows where this currently track id is located        
        // Get all the table columns and store them in the hash for this
        // row-info section.

        foreach (int tableColumnIndex, tableColumnIndices) {
            thisRowInfo.metadata[tableColumnIndex] =
                    query.value(tableColumnIndex);
        }
        rowInfo.push_back(thisRowInfo);
    }

    if (sDebug) {
        qDebug() << "Rows actually received:" << rowInfo.size();
    }

    // Adjust sort column to remove table columns and add 1 to add an id column.
    int sortColumn = m_iSortColumn - m_tableColumns.size() + 1;

    if (sortColumn < 0) {
        sortColumn = 0;
    }

    // If we were sorting a table column, then secondary sort by id. TODO(rryan)
    // we should look into being able to drop the secondary sort to save time
    // but going for correctness first.
    m_trackSource->filterAndSort(trackIds, m_currentSearch,
                                 m_currentSearchFilter,
                                 sortColumn, m_eSortOrder,
                                 &m_trackSortOrder);

    // Re-sort the track IDs since filterAndSort can change their order or mark
    // them for removal (by setting their row to -1).
    for (QVector<RowInfo>::iterator it = rowInfo.begin();
         it != rowInfo.end(); ++it) {
        // If the sort column is not a track column then we will sort only to
        // separate removed tracks (order == -1) from present tracks (order ==
        // 0). Otherwise we sort by the order that filterAndSort returned to us.
        if (sortColumn == 0) {
            it->order = m_trackSortOrder.contains(it->trackId) ? 0 : -1;
        } else {
            it->order = m_trackSortOrder.value(it->trackId, -1);
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
    beginInsertRows(QModelIndex(), 0, rowInfo.size()-1);
    m_rowInfo = rowInfo;
    m_bDirty = false;
    endInsertRows();

    int elapsed = time.elapsed();
    qDebug() << this << "select() took" << elapsed << "ms";
}

void BaseSqlTableModel::setTable(const QString& tableName,
                                 const QString& idColumn,
                                 const QStringList& tableColumns,
                                 QSharedPointer<BaseTrackCache> trackSource) {
    Q_ASSERT(trackSource);
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
    connect(m_trackSource.data(), SIGNAL(tracksChanged(QSet<int>)),
            this, SLOT(tracksChanged(QSet<int>)));

    // Build a map from the column names to their indices, used by fieldIndex()
    m_tableColumnIndex.clear();
    for (int i = 0; i < tableColumns.size(); ++i) {
        m_tableColumnIndex[m_tableColumns[i]] = i;
    }

    m_bInitialized = true;
    m_bDirty = true;
}

const QString BaseSqlTableModel::currentSearch() const {
    return m_currentSearch;
}

void BaseSqlTableModel::setSearch(const QString& searchText, const QString extraFilter) {
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
    m_bDirty = true;
}

void BaseSqlTableModel::search(const QString& searchText, const QString extraFilter) {
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
    m_bDirty = true;
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

int BaseSqlTableModel::fieldIndex(const QString& fieldName) const {
    int tableIndex = m_tableColumnIndex.value(fieldName, -1);
    if (tableIndex > -1) {
        return tableIndex;
    }
    // Subtract one from the fieldIndex() result to account for the id column
    return m_trackSource ? (m_tableColumns.size() +
                            m_trackSource->fieldIndex(fieldName) - 1) : -1;
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
            if (column == fieldIndex(LIBRARYTABLE_DURATION)) {
                if (qVariantCanConvert<int>(value)) {
                    value = MixxxUtils::secondsToMinutes(
                        qVariantValue<int>(value));
                }
            } else if (column == fieldIndex(LIBRARYTABLE_RATING)) {
                if (qVariantCanConvert<int>(value))
                    value = qVariantFromValue(StarRating(value.toInt()));
            } else if (column == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
                if (qVariantCanConvert<int>(value))
                    value =  QString("(%1)").arg(value.toInt());
            } else if (column == fieldIndex(LIBRARYTABLE_PLAYED)) {
                value = value.toBool();
            } else if (column == fieldIndex(LIBRARYTABLE_DATETIMEADDED)) {
                value = value.toDateTime();
            } else if (column == fieldIndex(PLAYLISTTRACKSTABLE_DATETIMEADDED)) {
                value = value.toDateTime().time();
            } else if (column == fieldIndex(LIBRARYTABLE_BPM_LOCK)) {
                value = value.toBool();
            }
            break;
        case Qt::EditRole:
            if (column == fieldIndex(LIBRARYTABLE_BPM)) {
                return value.toDouble();
            } else if (column == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
                return index.sibling(
                    row, fieldIndex(LIBRARYTABLE_PLAYED)).data().toBool();
            } else if (column == fieldIndex(LIBRARYTABLE_RATING)) {
                if (qVariantCanConvert<int>(value))
                    value = qVariantFromValue(StarRating(value.toInt()));
            }
            break;
        case Qt::CheckStateRole:
            if (column == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
                bool played = index.sibling(
                    row, fieldIndex(LIBRARYTABLE_PLAYED)).data().toBool();
                value = played ? Qt::Checked : Qt::Unchecked;
            } else if (column == fieldIndex(LIBRARYTABLE_BPM)) {
                bool locked = index.sibling(
                    row, fieldIndex(LIBRARYTABLE_BPM_LOCK)).data().toBool();
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
        if (column == fieldIndex(LIBRARYTABLE_TIMESPLAYED)) {
            QModelIndex playedIndex = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_PLAYED));
            return setData(playedIndex, val, Qt::EditRole);
        } else if (column == fieldIndex(LIBRARYTABLE_BPM)) {
            QModelIndex bpmLockindex = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_BPM_LOCK));
            return setData(bpmLockindex, val, Qt::EditRole);
        }
        return false;
    }

    if (row < 0 || row >= m_rowInfo.size()) {
        return false;
    }

    const RowInfo& rowInfo = m_rowInfo[row];
    int trackId = rowInfo.trackId;

    // You can't set something in the table columns because we have no way of
    // persisting it.
    const QHash<int, QVariant>& columns = rowInfo.metadata;
    if (columns.contains(column)) {
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

    if ( column == fieldIndex(LIBRARYTABLE_FILETYPE)
         || column == fieldIndex(LIBRARYTABLE_LOCATION)
         || column == fieldIndex(LIBRARYTABLE_DURATION)
         || column == fieldIndex(LIBRARYTABLE_BITRATE)
         || column == fieldIndex(LIBRARYTABLE_DATETIMEADDED)) {
        return defaultFlags;
    } else if (column == fieldIndex(LIBRARYTABLE_TIMESPLAYED))  {
        return defaultFlags | Qt::ItemIsUserCheckable;
    } else if (column == fieldIndex(LIBRARYTABLE_BPM_LOCK)) {
        return defaultFlags | Qt::ItemIsUserCheckable;
    } else if(column == fieldIndex(LIBRARYTABLE_BPM)) {
        // Allow checking of the BPM-locked indicator.
        defaultFlags |= Qt::ItemIsUserCheckable;
        // Disable editing of BPM field when BPM is locked
        bool locked = index.sibling(
            index.row(), fieldIndex(LIBRARYTABLE_BPM_LOCK))
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
    if (m_trackIdToRows.contains(trackId)) {
        return m_trackIdToRows[trackId];
    }
    return QLinkedList<int>();
}

int BaseSqlTableModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex(m_idColumn)).data().toInt();
}

QString BaseSqlTableModel::getTrackLocation(const QModelIndex& index) const {
    if (!index.isValid()) {
        return "";
    }
    QString location = index.sibling(
        index.row(), fieldIndex("location")).data().toString();
    return location;
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
    if (fieldIndex(LIBRARYTABLE_ARTIST) == column) {
        pTrack->setArtist(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_TITLE) == column) {
        pTrack->setTitle(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_ALBUM) == column) {
        pTrack->setAlbum(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_YEAR) == column) {
        pTrack->setYear(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_GENRE) == column) {
        pTrack->setGenre(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_COMPOSER) == column) {
        pTrack->setComposer(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_FILETYPE) == column) {
        pTrack->setType(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_TRACKNUMBER) == column) {
        pTrack->setTrackNumber(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_LOCATION) == column) {
        pTrack->setLocation(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_COMMENT) == column) {
        pTrack->setComment(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_DURATION) == column) {
        pTrack->setDuration(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_BITRATE) == column) {
        pTrack->setBitrate(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_BPM) == column) {
        // QVariant::toFloat needs >= QT 4.6.x
        pTrack->setBpm(static_cast<float>(value.toDouble()));
    } else if (fieldIndex(LIBRARYTABLE_PLAYED) == column) {
        pTrack->setPlayedAndUpdatePlaycount(value.toBool());
    } else if (fieldIndex(LIBRARYTABLE_TIMESPLAYED) == column) {
        pTrack->setTimesPlayed(value.toInt());
    } else if (fieldIndex(LIBRARYTABLE_RATING) == column) {
        StarRating starRating = qVariantValue<StarRating>(value);
        pTrack->setRating(starRating.starCount());
    } else if (fieldIndex(LIBRARYTABLE_KEY) == column) {
        pTrack->setKey(value.toString());
    } else if (fieldIndex(LIBRARYTABLE_BPM_LOCK) == column) {
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

    if (row < 0 || row >= m_rowInfo.size()) {
        return QVariant();
    }

    // TODO(rryan) check range on column

    const RowInfo& rowInfo = m_rowInfo[row];
    int trackId = rowInfo.trackId;

    // If the row info has the row-specific column, return that.
    const QHash<int, QVariant>& columns = rowInfo.metadata;
    if (columns.contains(column)) {
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
        QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
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
    if (i == fieldIndex(LIBRARYTABLE_RATING)) {
        return new StarDelegate(pParent);
    }
    return NULL;
}

void BaseSqlTableModel::hideTracks(const QModelIndexList& indices) {
    QList<int> trackIds;
    foreach (QModelIndex index, indices) {
        int trackId = getTrackId(index);
        trackIds.append(trackId);
    }

    m_trackDAO.hideTracks(trackIds);

    // TODO(rryan) : do not select, instead route event to BTC and notify from
    // there.
    select(); //Repopulate the data model.
}


