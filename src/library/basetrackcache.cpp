#include "library/basetrackcache.h"

#include "library/queryutil.h"
#include "library/searchquery.h"
#include "library/searchqueryparser.h"
#include "library/trackcollection.h"
#include "moc_basetrackcache.cpp"
#include "track/globaltrackcache.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "util/performancetimer.h"

namespace {

constexpr bool sDebug = false;

}  // namespace

BaseTrackCache::BaseTrackCache(TrackCollection* pTrackCollection,
        QString tableName,
        QString idColumn,
        QStringList columns,
        QStringList searchColumns,
        bool isCaching)
        : m_tableName(std::move(tableName)),
          m_idColumn(std::move(idColumn)),
          m_columnCount(columns.size()),
          m_columnsJoined(columns.join(",")),
          m_columnCache(std::move(columns)),
          m_pQueryParser(std::make_unique<SearchQueryParser>(
                  pTrackCollection, std::move(searchColumns))),
          m_bIndexBuilt(false),
          m_bIsCaching(isCaching),
          m_database(pTrackCollection->database()) {
}

BaseTrackCache::~BaseTrackCache() {
    // Required to allow forward declarations of (managed pointer) members
    // in header file
}

int BaseTrackCache::columnCount() const {
    return m_columnCount;
}

int BaseTrackCache::fieldIndex(ColumnCache::Column column) const {
    return m_columnCache.fieldIndex(column);
}

int BaseTrackCache::fieldIndex(const QString& columnName) const {
    return m_columnCache.fieldIndex(columnName);
}

int BaseTrackCache::endFieldIndex() const {
    return m_columnCache.endFieldIndex();
}

QString BaseTrackCache::columnNameForFieldIndex(int index) const {
    return m_columnCache.columnNameForFieldIndex(index);
}

QString BaseTrackCache::columnSortForFieldIndex(int index) const {
    return m_columnCache.columnSortForFieldIndex(index);
}

void BaseTrackCache::slotTracksAddedOrChanged(const QSet<TrackId>& trackIds) {
    if (sDebug) {
        qDebug() << this << "slotTracksAddedOrChanged" << trackIds.size();
    }
    updateTracksInIndex(trackIds);
}

void BaseTrackCache::slotScanTrackAdded(TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "slotScanTrackAdded";
    }
    updateTrackInIndex(pTrack);
}

void BaseTrackCache::slotTracksRemoved(const QSet<TrackId>& trackIds) {
    if (sDebug) {
        qDebug() << this << "slotTracksRemoved" << trackIds.size();
    }
    for (const auto& trackId : std::as_const(trackIds)) {
        m_trackInfo.remove(trackId);
        m_dirtyTracks.remove(trackId);
    }
}

void BaseTrackCache::slotTrackDirty(TrackId trackId) {
    if (sDebug) {
        qDebug() << this << "slotTrackDirty" << trackId;
    }
    m_dirtyTracks.insert(trackId);
}

void BaseTrackCache::slotTrackClean(TrackId trackId) {
    if (sDebug) {
        qDebug() << this << "slotTrackClean" << trackId;
    }
    m_dirtyTracks.remove(trackId);
    // The track might have been reloaded from the database
    updateTrackInIndex(trackId);
}

bool BaseTrackCache::isCached(TrackId trackId) const {
    return m_trackInfo.contains(trackId);
}

void BaseTrackCache::ensureCached(TrackId trackId) {
    updateTrackInIndex(trackId);
}

const TrackPointer& BaseTrackCache::getCachedTrack(TrackId trackId) const {
    DEBUG_ASSERT(m_bIsCaching);
    // Only refresh the recently used track if the identifiers
    // don't match. Otherwise simply return the corresponding
    // pointer to avoid accessing and locking the global track
    // cache excessively.
    if (m_recentTrackId != trackId) {
        if (trackId.isValid()) {
            TrackPointer trackPtr =
                    GlobalTrackCacheLocker().lookupTrackById(trackId);
            if (!trackPtr) {
                resetRecentTrack();
            } else {
                replaceRecentTrack(
                        std::move(trackId),
                        std::move(trackPtr));
            }
        }
    }
    return m_recentTrackPtr;
}

void BaseTrackCache::replaceRecentTrack(TrackPointer pTrack) const {
    DEBUG_ASSERT(m_bIsCaching);
    DEBUG_ASSERT(pTrack);
    // Temporary needed, because std::move invalidates the smart pointer
    auto trackId = pTrack->getId();
    replaceRecentTrack(std::move(trackId), std::move(pTrack));
}

void BaseTrackCache::replaceRecentTrack(TrackId trackId, TrackPointer pTrack) const {
    // reset recent track first, because that may evict if from GlobalTrackCache cache
    // causing updateIndexWithQuery() which resets the recent track again.
    resetRecentTrack();
    DEBUG_ASSERT(!pTrack || m_recentTrackId != pTrack->getId());
    m_recentTrackId = std::move(trackId);
    m_recentTrackPtr = std::move(pTrack);
}

void BaseTrackCache::resetRecentTrack() const {
    m_recentTrackId = TrackId();
    m_recentTrackPtr.reset();
}

bool BaseTrackCache::updateTrackInIndex(
        const TrackPointer& pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return false;
    }
    if (sDebug) {
        qDebug() << "updateTrackInIndex:" << pTrack->getLocation();
    }

    int numColumns = columnCount();

    TrackId trackId = pTrack->getId();
    if (trackId.isValid()) {
        // m_trackInfo[id] will insert a QVector<QVariant> into the
        // m_trackInfo HashTable with the key "id"
        QVector<QVariant>& record = m_trackInfo[trackId];
        // preallocate memory for all columns at once
        record.resize(numColumns);
        for (int i = 0; i < numColumns; ++i) {
            record[i] = getTrackValueForColumn(pTrack, i);
        }
        if (m_bIsCaching) {
            replaceRecentTrack(trackId, pTrack);
        }
    } else {
        if (m_bIsCaching) {
            resetRecentTrack();
        }
    }
    return true;
}

bool BaseTrackCache::updateIndexWithQuery(const QString& queryString) {
    PerformanceTimer timer;
    timer.start();

    if (sDebug) {
        qDebug() << "updateIndexWithQuery issuing query:" << queryString;
    }

    QSqlQuery query(m_database);
    // This causes a memory savings since QSqlCachedResult (what QtSQLite uses)
    // won't allocate a giant in-memory table that we won't use at all.
    query.setForwardOnly(true); // performance improvement?
    query.prepare(queryString);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    int numColumns = columnCount();
    int idColumn = query.record().indexOf(m_idColumn);

    while (query.next()) {
        TrackId trackId(query.value(idColumn));

        //m_trackInfo[id] will insert a QVector<QVariant> into the
        //m_trackInfo HashTable with the key "id"
        QVector<QVariant>& record = m_trackInfo[trackId];
        record.resize(numColumns);

        for (int i = 0; i < numColumns; ++i) {
            if (fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION) == i) {
                // Database stores all locations with Qt separators: "/"
                // Here we want to cache the display string with native separators.
                QString location = query.value(i).toString();
                record[i] = QDir::toNativeSeparators(location);
            } else {
                record[i] = query.value(i);
            }
        }
    }

    qDebug() << this << "updateIndexWithQuery took" << timer.elapsed().debugMillisWithUnit();
    return true;
}

void BaseTrackCache::buildIndex() {
    if (sDebug) {
        qDebug() << this << "buildIndex()";
    }

    QString queryString = QString("SELECT %1 FROM %2")
            .arg(m_columnsJoined, m_tableName);

    if (sDebug) {
        qDebug() << this << "buildIndex query:" << queryString;
    }

    // TODO(rryan) for very large tables, it probably makes more sense to NOT
    // clear the table, and keep track of what IDs we see, then delete the ones
    // we don't see.
    m_trackInfo.clear();
    if (m_bIsCaching) {
        resetRecentTrack();
    }

    if (!updateIndexWithQuery(queryString)) {
        qDebug() << "buildIndex failed!";
    }

    m_bIndexBuilt = true;
}

void BaseTrackCache::updateTrackInIndex(TrackId trackId) {
    QSet<TrackId> trackIds;
    trackIds.insert(trackId);
    updateTracksInIndex(trackIds);
}

void BaseTrackCache::updateTracksInIndex(const QSet<TrackId>& trackIds) {
    if (trackIds.isEmpty()) {
        return;
    }

    QStringList idStrings;
    idStrings.reserve(trackIds.size());
    for (const auto& trackId: trackIds) {
        idStrings << trackId.toString();
    }

    QString queryString = QString("SELECT %1 FROM %2 WHERE %3 in (%4)")
            .arg(m_columnsJoined, m_tableName, m_idColumn, idStrings.join(","));

    if (sDebug) {
        qDebug() << this << "updateTracksInIndex update query:" << queryString;
    }

    if (!updateIndexWithQuery(queryString)) {
        qDebug() << "updateTracksInIndex failed!";
        return;
    }
    emit tracksChanged(trackIds);
}

QVariant BaseTrackCache::getTrackValueForColumn(TrackPointer pTrack,
        int column) const {
    if (!pTrack || column < 0) {
        return QVariant{};
    }

    if (m_bIsCaching) {
        replaceRecentTrack(pTrack);
    }

    // TODO(XXX) Qt properties could really help here.
    // TODO(rryan) this is all TrackDAO specific. What about iTunes/RB/etc.?
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST) == column) {
        return QVariant{pTrack->getArtist()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE) == column) {
        return QVariant{pTrack->getTitle()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM) == column) {
        return QVariant{pTrack->getAlbum()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST) == column) {
        return QVariant{pTrack->getAlbumArtist()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR) == column) {
        return QVariant{pTrack->getYear()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) == column) {
        return QVariant{pTrack->getDateAdded()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT) == column) {
        return QVariant{pTrack->getLastPlayedAt()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE) == column) {
        return QVariant{pTrack->getGenre()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER) == column) {
        return QVariant{pTrack->getComposer()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING) == column) {
        return QVariant{pTrack->getGrouping()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) == column) {
        return QVariant{pTrack->getType()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) == column) {
        return QVariant{pTrack->getTrackNumber()};
    }
    if (fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION) == column) {
        return QVariant{QDir::toNativeSeparators(pTrack->getLocation())};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) == column) {
        return QVariant{pTrack->getComment()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) == column) {
        return QVariant{pTrack->getDuration()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) == column) {
        return QVariant{pTrack->getBitrate()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM) == column) {
        return QVariant{pTrack->getBpm()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN) == column) {
        return QVariant{pTrack->getReplayGain().getRatio()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) == column) {
        return QVariant{pTrack->getPlayCounter().isPlayed()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) == column) {
        return QVariant{pTrack->getPlayCounter().getTimesPlayed()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING) == column) {
        return QVariant{pTrack->getRating()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY) == column) {
        // The Key value is determined by either the KEY_ID or KEY column
        return QVariant{KeyUtils::keyFromKeyTextAndIdValues(
                pTrack->getKeyText(), pTrack->getKey())};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) == column) {
        return QVariant{static_cast<int>(pTrack->getKey())};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) == column) {
        return QVariant{pTrack->isBpmLocked()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR) == column) {
        return mixxx::RgbColor::toQVariant(pTrack->getColor());
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) == column) {
        return QVariant{pTrack->getCoverInfo().coverLocation};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH) == column ||
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART) == column) {
        // For sorting, we give COLUMN_LIBRARYTABLE_COVERART the same value as
        // the cover digest.
        return QVariant{pTrack->getCoverInfo().imageDigest()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR) == column) {
        return mixxx::RgbColor::toQVariant(pTrack->getCoverInfo().color);
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_DIGEST) == column) {
        return QVariant{pTrack->getCoverInfo().imageDigest()};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) == column) {
        return QVariant{static_cast<int>(pTrack->getCoverInfo().source)};
    }
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) == column) {
        return QVariant{static_cast<int>(pTrack->getCoverInfo().type)};
    }
    return QVariant{};
}

QVariant BaseTrackCache::data(TrackId trackId, int column) const {
    if (!m_bIndexBuilt) {
        qDebug() << this << "ERROR index is not built for" << m_tableName;
        return QVariant{};
    }

    if (m_bIsCaching) {
        TrackPointer pTrack = getCachedTrack(trackId);
        if (pTrack) {
            QVariant result = getTrackValueForColumn(pTrack, column);
            if (result.isValid()) {
                return result;
            }
        }
    }

    // If the track lookup failed (could happen for track properties we don't
    // keep track of in Track, like playlist position) look up the value in
    // the track info cache.

    // TODO(rryan) this code is flawed for columns that contains row-specific
    // metadata. Currently the upper-levels will not delegate row-specific
    // columns to this method, but there should still be a check here I think.
    auto it = m_trackInfo.constFind(trackId);
    if (it == m_trackInfo.constEnd()) {
        return QVariant{};
    }

    const QVector<QVariant>& fields = it.value();
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY)) {
        // The Key value is determined by either the KEY_ID or KEY column
        const auto columnForKeyId = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID);
        return KeyUtils::keyFromKeyTextAndIdFields(
                fields.value(column, QVariant{}),
                fields.value(columnForKeyId, QVariant{}));
    }
    return fields.value(column, QVariant{});
}

void BaseTrackCache::filterAndSort(const QSet<TrackId>& trackIds,
                                   const QString& searchQuery,
                                   const QString& extraFilter,
                                   const QString& orderByClause,
                                   const QList<SortColumn>& sortColumns,
                                   const int columnOffset,
                                   QHash<TrackId, int>* trackToIndex) {
    // Skip processing if there are no tracks to filter or sort.
    if (trackIds.size() == 0) {
        return;
    }

    if (!m_bIndexBuilt) {
        buildIndex();
    }

    QStringList idStrings;
    // TODO(rryan) consider making this the data passed in and a separate
    // QVector for output
    QSet<TrackId> dirtyTracks;
    for (const auto& trackId: trackIds) {
        idStrings << trackId.toString();
        if (m_dirtyTracks.contains(trackId)) {
            dirtyTracks.insert(trackId);
        }
    }

    QStringList queryFragments;
    if (!extraFilter.isNull() && extraFilter != "") {
        queryFragments << QString("(%1)").arg(extraFilter);
    }
    if (idStrings.size() > 0) {
        queryFragments << QString("%1 in (%2)")
                .arg(m_idColumn, idStrings.join(","));
    }

    const std::unique_ptr<QueryNode> pQuery =
            m_pQueryParser->parseQuery(
                    searchQuery,
                    queryFragments.join(" AND "));

    QString filter = pQuery->toSql();
    if (!filter.isEmpty()) {
        filter.prepend("WHERE ");
    }

    QString queryString = QString("SELECT %1 FROM %2 %3 %4")
            .arg(m_idColumn, m_tableName, filter, orderByClause);

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
    }

    int idColumn = query.record().indexOf(m_idColumn);
    int rows = query.size();

    if (sDebug) {
        qDebug() << "Rows returned:" << rows;
    }

    m_trackOrder.resize(0); // keeps allocated memory
    trackToIndex->clear();
    if (rows > 0) {
        trackToIndex->reserve(rows);
        m_trackOrder.reserve(rows);
    }

    while (query.next()) {
        TrackId trackId(query.value(idColumn));
        (*trackToIndex)[trackId] = m_trackOrder.size();
        m_trackOrder.append(trackId);
    }

    // At this point, the original set of tracks have been divided into two
    // pieces: those that should be in the result set and those that should
    // not. Unfortunately, due to TrackDAO caching, there may be tracks in
    // either category that are there incorrectly. We must look at all the dirty
    // tracks (within the original set, if specified) and evaluate whether they
    // would match or not match the given filter criteria. Once we correct the
    // membership of tracks in either set, we must then insertion-sort the
    // missing tracks into the resulting index list.

    if (!m_bIsCaching || dirtyTracks.isEmpty()) {
        return;
    }

    for (TrackId trackId : std::as_const(dirtyTracks)) {
        // Only get the track if it is in the cache. Tracks that
        // are not cached in memory cannot be dirty.
        // Bypass getCachedTrack() to not invalidate m_recentTrackId
        TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
        if (!pTrack) {
            continue;
        }

        // The track should be in the result set if the search is empty or the
        // track matches the search.
        bool shouldBeInResultSet = searchQuery.isEmpty() ||
                pQuery->match(pTrack);

        // If the track is in this result set.
        bool isInResultSet = trackToIndex->contains(trackId);

        if (shouldBeInResultSet) {
            // Track should be in result set...

            // Remove the track from the results first (we have to do this or it
            // will sort wrong).
            if (isInResultSet) {
                int index = (*trackToIndex)[trackId];
                m_trackOrder.remove(index);
                // Don't update trackToIndex, since we do it below.
            }

            // Figure out where it is supposed to sort. The table is sorted by
            // the sort column, so we can binary search.
            int insertRow = findSortInsertionPoint(
                    pTrack, sortColumns, columnOffset, m_trackOrder);

            if (sDebug) {
                qDebug() << this
                         << "Insertion sort says it should be inserted at:"
                         << insertRow;
            }

            // The track should sort at insertRow
            m_trackOrder.insert(insertRow, trackId);

            trackToIndex->clear();
            // Fix the index. TODO(rryan) find a non-stupid way to do this.
            for (int i = 0; i < m_trackOrder.size(); ++i) {
                (*trackToIndex)[m_trackOrder[i]] = i;
            }
        } else if (isInResultSet) {
            // Track should not be in this result set, but it is. We need to
            // remove it.
            int index = (*trackToIndex)[trackId];
            m_trackOrder.remove(index);

            trackToIndex->clear();
            // Fix the index. TODO(rryan) find a non-stupid way to do this.
            for (int i = 0; i < m_trackOrder.size(); ++i) {
                (*trackToIndex)[m_trackOrder[i]] = i;
            }
        }
    }
}

int BaseTrackCache::findSortInsertionPoint(TrackPointer pTrack,
        const QList<SortColumn>& sortColumns,
        const int columnOffset,
        const QVector<TrackId>& trackIds) const {
    QList<QVariant> trackValues;
    if (sortColumns.isEmpty()) {
        return 0;
    }
    for (const auto& sc: sortColumns) {
        trackValues.append(getTrackValueForColumn(pTrack, sc.m_column - columnOffset));
    }

    int min = 0;
    int max = trackIds.size() - 1;

    if (sDebug) {
        qDebug() << this << "Trying to insertion sort:"
                 << trackValues.at(0) << "min" << min << "max" << max;
    }

    // If trackIds is empty, min is 0 and max is -1 so findSortInsertionPoint
    // returns 0.
    while (min <= max) {
        int mid = min + (max - min) / 2;
        TrackId otherTrackId(trackIds[mid]);

        // This should not happen, but it's a recoverable error so we should
        // only log it.
        if (!m_trackInfo.contains(otherTrackId)) {
            qDebug() << "WARNING: track" << otherTrackId << "was not in index";
            //updateTrackInIndex(otherTrackId);
        }

        int compare = 0;
        for (int i = 0; i < sortColumns.count(); i++) {
            QVariant tableValue =
                    data(otherTrackId, sortColumns[i].m_column - columnOffset);

            compare = compareColumnValues(
                    sortColumns[i].m_column - columnOffset,
                    sortColumns[i].m_order,
                    trackValues[i],
                    tableValue);

            if (compare != 0) {
                break;
            }
        }

        if (compare == 0) {
            // Alright, if we're here then we can insert it here and be
            // "correct"
            min = mid;
            break;
        } else if (compare > 0) {
            min = mid + 1;
        } else {
            max = mid - 1;
        }
    }
    return min;
}

int BaseTrackCache::compareColumnValues(int sortColumn,
        Qt::SortOrder sortOrder,
        const QVariant& val1,
        const QVariant& val2) const {
    int result = 0;

    if (sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_SAMPLERATE) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_CHANNELS) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING) ||
            sortColumn == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION)
    ) {
        // Sort as floats.
        double delta = val1.toDouble() - val2.toDouble();

        if (fabs(delta) < .00001) {
            result = 0;
        } else if (delta > 0.0) {
            result = 1;
        } else {
            result = -1;
        }
    } else if (sortColumn == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY)) {
        KeyUtils::KeyNotation keyNotation = m_columnCache.keyNotation();

        int key1 = KeyUtils::keyToCircleOfFifthsOrder(
            KeyUtils::guessKeyFromText(val1.toString()), keyNotation);
        int key2 = KeyUtils::keyToCircleOfFifthsOrder(
            KeyUtils::guessKeyFromText(val2.toString()), keyNotation);
        if (key1 > key2) {
            result = 1;
        } else if (key1 < key2) {
            result = -1;
        } else if (key1 == key2) {
            result = 0;
        }
    } else {
        result = m_collator.compare(val1.toString(), val2.toString());
    }

    // If we're in descending order, flip the comparison.
    if (sortOrder == Qt::DescendingOrder) {
        result = -result;
    }

    return result;
}
