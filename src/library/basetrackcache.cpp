// basetrackcache.cpp
// Created 7/3/2011 by RJ Ryan (rryan@mit.edu)

#include "library/basetrackcache.h"

#include "library/trackcollection.h"
#include "library/searchqueryparser.h"
#include "library/queryutil.h"
#include "track/keyutils.h"
#include "track/globaltrackcache.h"
#include "util/performancetimer.h"
#include "util/compatibility.h"

namespace {

constexpr bool sDebug = false;

}  // namespace

BaseTrackCache::BaseTrackCache(TrackCollection* pTrackCollection,
                               const QString& tableName,
                               const QString& idColumn,
                               const QStringList& columns,
                               bool isCaching)
        : QObject(),
          m_tableName(tableName),
          m_idColumn(idColumn),
          m_columnCount(columns.size()),
          m_columnsJoined(columns.join(",")),
          m_columnCache(columns),
          m_bIndexBuilt(false),
          m_bIsCaching(isCaching),
          m_trackDAO(pTrackCollection->getTrackDAO()),
          m_database(pTrackCollection->database()),
          m_pQueryParser(new SearchQueryParser(pTrackCollection)) {
    m_searchColumns << "artist"
                    << "album"
                    << "album_artist"
                    << "location"
                    << "grouping"
                    << "comment"
                    << "title"
                    << "genre"
                    << "crate";

    // Convert all the search column names to their field indexes because we use
    // them a bunch.
    m_searchColumnIndices.resize(m_searchColumns.size());
    for (int i = 0; i < m_searchColumns.size(); ++i) {
        m_searchColumnIndices[i] = m_columnCache.fieldIndex(m_searchColumns[i]);
    }
}

BaseTrackCache::~BaseTrackCache() {
    delete m_pQueryParser;
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

QString BaseTrackCache::columnNameForFieldIndex(int index) const {
    return m_columnCache.columnNameForFieldIndex(index);
}

QString BaseTrackCache::columnSortForFieldIndex(int index) const {
    return m_columnCache.columnSortForFieldIndex(index);
}

void BaseTrackCache::slotTracksAdded(QSet<TrackId> trackIds) {
    if (sDebug) {
        qDebug() << this << "slotTracksAdded" << trackIds.size();
    }
    QSet<TrackId> updateTrackIds;
    for (const auto& trackId: qAsConst(trackIds)) {
        updateTrackIds.insert(trackId);
    }
    updateTracksInIndex(updateTrackIds);
}

void BaseTrackCache::slotDbTrackAdded(TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "slotDbTrackAdded";
    }
    updateIndexWithTrackpointer(pTrack);
}

void BaseTrackCache::slotTracksRemoved(QSet<TrackId> trackIds) {
    if (sDebug) {
        qDebug() << this << "slotTracksRemoved" << trackIds.size();
    }
    for (const auto& trackId : qAsConst(trackIds)) {
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

void BaseTrackCache::slotTrackChanged(TrackId trackId) {
    if (sDebug) {
        qDebug() << this << "slotTrackChanged" << trackId;
    }
    QSet<TrackId> trackIds;
    trackIds.insert(trackId);
    emit(tracksChanged(trackIds));
}

void BaseTrackCache::slotTrackClean(TrackId trackId) {
    if (sDebug) {
        qDebug() << this << "slotTrackClean" << trackId;
    }
    m_dirtyTracks.remove(trackId);
    updateTrackInIndex(trackId);
}

bool BaseTrackCache::isCached(TrackId trackId) const {
    return m_trackInfo.contains(trackId);
}

void BaseTrackCache::ensureCached(TrackId trackId) {
    updateTrackInIndex(trackId);
}

void BaseTrackCache::ensureCached(QSet<TrackId> trackIds) {
    updateTracksInIndex(trackIds);
}

void BaseTrackCache::setSearchColumns(const QStringList& columns) {
    m_searchColumns = columns;
}

const TrackPointer& BaseTrackCache::getRecentTrack(TrackId trackId) const {
    DEBUG_ASSERT(m_bIsCaching);
    // Only refresh the recently used track if the identifiers
    // don't match. Otherwise simply return the corresponding
    // pointer to avoid accessing and locking the global track
    // cache excessively.
    if (m_recentTrackId != trackId) {
        if (trackId.isValid()) {
            TrackPointer trackPtr =
                    GlobalTrackCacheLocker().lookupTrackById(trackId);
            replaceRecentTrack(
                    std::move(trackId),
                    std::move(trackPtr));
        } else {
            resetRecentTrack();
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
    DEBUG_ASSERT(m_bIsCaching);
    m_recentTrackId = std::move(trackId);
    if (m_recentTrackId.isValid()) {
        if (pTrack) {
            DEBUG_ASSERT(m_recentTrackId == pTrack->getId());

            // NOTE(uklotzde, 2018-02-07, Fedora 27, GCC 7.3.1, optimize=native (-O3), Core i5-6440HQ)
            // Using the move assignment operator here will sooner or later
            // store a nullptr in m_recentTrackPtr even if both m_recentTrackPtr
            // and pTrack contain a valid but different pointer before the
            // assignment and the custom deleter is invoked on m_recentTrackPtr
            // during the assignment. The issue does not occur when either
            // changing the optimization level from 'native' to 'none' or
            // when replacing the move assignment with a swap operation (see below).
            //
            // Fucked up by the optimizer:
            // m_recentTrackPtr = std::move(pTrack);
            //
            // The workaround:
            m_recentTrackPtr.swap(pTrack);
            //
            // The following debug assertion will be triggered eventually
            // without the workaround in place when browsing and editing
            // properties of tracks.
            DEBUG_ASSERT(m_recentTrackPtr);

            if (m_recentTrackPtr->isDirty()) {
                m_dirtyTracks.insert(m_recentTrackId);
            } else {
                m_dirtyTracks.remove(m_recentTrackId);
            }
        } else {
            // The track cannot be dirty if it is not present
            m_recentTrackPtr.reset();
            m_dirtyTracks.remove(m_recentTrackId);
        }
    } else {
        DEBUG_ASSERT(!pTrack);
        m_recentTrackPtr.reset();
    }
}

void BaseTrackCache::resetRecentTrack() const {
    DEBUG_ASSERT(m_bIsCaching);
    m_recentTrackId = TrackId();
    m_recentTrackPtr.reset();
}

bool BaseTrackCache::updateIndexWithTrackpointer(TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << "updateIndexWithTrackpointer:" << pTrack->getFileInfo();
    }

    if (!pTrack) {
        return false;
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
            getTrackValueForColumn(pTrack, i, record[i]);
        }
        if (m_bIsCaching) {
            replaceRecentTrack(std::move(trackId), std::move(pTrack));
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

    if (m_bIsCaching) {
        resetRecentTrack();
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
            if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION) == i) {
                // Database stores all locations with Qt separators: "/"
                // Here we want to cache the display string with native separators.
                QString location = query.value(i).toString();
                record[i] = QDir::toNativeSeparators(location);
            }
            else {
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
    emit(tracksChanged(trackIds));
}

void BaseTrackCache::getTrackValueForColumn(TrackPointer pTrack,
                                            int column,
                                            QVariant& trackValue) const {
    if (!pTrack || column < 0) {
        return;
    }

    if (m_bIsCaching) {
        replaceRecentTrack(pTrack);
    }

    // TODO(XXX) Qt properties could really help here.
    // TODO(rryan) this is all TrackDAO specific. What about iTunes/RB/etc.?
    if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST) == column) {
        trackValue.setValue(pTrack->getArtist());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE) == column) {
        trackValue.setValue(pTrack->getTitle());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM) == column) {
        trackValue.setValue(pTrack->getAlbum());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST) == column) {
        trackValue.setValue(pTrack->getAlbumArtist());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR) == column) {
        trackValue.setValue(pTrack->getYear());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED) == column) {
        trackValue.setValue(pTrack->getDateAdded());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE) == column) {
        trackValue.setValue(pTrack->getGenre());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER) == column) {
        trackValue.setValue(pTrack->getComposer());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING) == column) {
        trackValue.setValue(pTrack->getGrouping());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE) == column) {
        trackValue.setValue(pTrack->getType());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER) == column) {
        trackValue.setValue(pTrack->getTrackNumber());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION) == column) {
        trackValue.setValue(QDir::toNativeSeparators(pTrack->getLocation()));
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT) == column) {
        trackValue.setValue(pTrack->getComment());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION) == column) {
        trackValue.setValue(pTrack->getDuration());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) == column) {
        trackValue.setValue(pTrack->getBitrate());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM) == column) {
        trackValue.setValue(pTrack->getBpm());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN) == column) {
        trackValue.setValue(pTrack->getReplayGain().getRatio());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED) == column) {
        trackValue.setValue(pTrack->getPlayCounter().isPlayed());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED) == column) {
        trackValue.setValue(pTrack->getPlayCounter().getTimesPlayed());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING) == column) {
        trackValue.setValue(pTrack->getRating());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY) == column) {
        trackValue.setValue(pTrack->getKeyText());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID) == column) {
        trackValue.setValue(static_cast<int>(pTrack->getKey()));
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) == column) {
        trackValue.setValue(pTrack->isBpmLocked());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_LOCATION) == column) {
        trackValue.setValue(pTrack->getCoverInfo().coverLocation);
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_HASH) == column ||
               fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART) == column) {
        // For sorting, we give COLUMN_LIBRARYTABLE_COVERART the same value as
        // the cover hash.
        trackValue.setValue(pTrack->getCoverHash());
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_SOURCE) == column) {
        trackValue.setValue(static_cast<int>(pTrack->getCoverInfo().source));
    } else if (fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_TYPE) == column) {
        trackValue.setValue(static_cast<int>(pTrack->getCoverInfo().type));
    }
}

QVariant BaseTrackCache::data(TrackId trackId, int column) const {
    QVariant result;

    if (!m_bIndexBuilt) {
        qDebug() << this << "ERROR index is not built for" << m_tableName;
        return result;
    }

    if (m_bIsCaching) {
        TrackPointer pTrack = getRecentTrack(trackId);
        if (pTrack) {
            getTrackValueForColumn(pTrack, column, result);
        }
    }

    // If the track lookup failed (could happen for track properties we don't
    // keep track of in Track, like playlist position) look up the value in
    // the track info cache.

    // TODO(rryan) this code is flawed for columns that contains row-specific
    // metadata. Currently the upper-levels will not delegate row-specific
    // columns to this method, but there should still be a check here I think.
    if (!result.isValid()) {
        auto it = m_trackInfo.constFind(trackId);
        if (it != m_trackInfo.constEnd()) {
            const QVector<QVariant>& fields = it.value();
            result = fields.value(column, result);
        }
    }
    return result;
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

    std::unique_ptr<QueryNode> pQuery(parseQuery(
        searchQuery, extraFilter, idStrings));

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

    for (TrackId trackId: qAsConst(dirtyTracks)) {
        // Only get the track if it is in the cache. Tracks that
        // are not cached in memory cannot be dirty.
        TrackPointer pTrack = getRecentTrack(trackId);

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

std::unique_ptr<QueryNode> BaseTrackCache::parseQuery(QString query, QString extraFilter,
                                      QStringList idStrings) const {
    QStringList queryFragments;
    if (!extraFilter.isNull() && extraFilter != "") {
        queryFragments << QString("(%1)").arg(extraFilter);
    }

    if (idStrings.size() > 0) {
        queryFragments << QString("%1 in (%2)")
                .arg(m_idColumn, idStrings.join(","));
    }

    return m_pQueryParser->parseQuery(query, m_searchColumns,
                                      queryFragments.join(" AND "));
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
        QVariant trackValue;
        getTrackValueForColumn(pTrack, sc.m_column - columnOffset, trackValue);
        trackValues.append(trackValue);
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

int BaseTrackCache::compareColumnValues(int sortColumn, Qt::SortOrder sortOrder,
                                        QVariant val1, QVariant val2) const {
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

        if (fabs(delta) < .00001)
            result = 0;
        else if (delta > 0.0)
            result = 1;
        else
            result = -1;
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
