#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>

#include "library/columncache.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/class.h"
#include "util/string.h"

class SearchQueryParser;
class TrackCollection;

class SortColumn {
  public:
    SortColumn(int column, Qt::SortOrder order)
        : m_column(column),
          m_order(order) {
    }
    int m_column;
    Qt::SortOrder m_order;
};

// BaseTrackCache is a cache of all of the values in certain table. It supports
// searching and sorting of tracks by values within the table. The reasoning for
// this is that previously there was a per-table-model cache which was largely a
// waste of memory because all the table-models were caching the same data
// (track properties). Furthermore, the base SQL tables of these table-models
// involve complicated joins, which are very slow.
class BaseTrackCache : public QObject {
    Q_OBJECT
  public:
    /// Construct a BaseTrackCache object.
    ///
    /// The order of the `columns` list parameter defines the initial/default
    /// order of columns in the library view.
    BaseTrackCache(TrackCollection* pTrackCollection,
            QString tableName,
            QString idColumn,
            QStringList columns,
            QStringList searchColumns,
            bool isCaching);
    ~BaseTrackCache() override;

    // Rebuild the BaseTrackCache index from the SQL table. This can be
    // expensive on large tables.
    virtual void buildIndex();

    ////////////////////////////////////////////////////////////////////////////
    // Data access methods
    ////////////////////////////////////////////////////////////////////////////

    virtual QVariant data(TrackId trackId, int column) const;
    virtual int columnCount() const;
    virtual int fieldIndex(const QString& column) const;
    QString columnNameForFieldIndex(int index) const;
    QString columnSortForFieldIndex(int index) const;
    int fieldIndex(ColumnCache::Column column) const;
    virtual void filterAndSort(const QSet<TrackId>& trackIds,
                               const QString& query,
                               const QString& extraFilter,
                               const QString& orderByClause,
                               const QList<SortColumn>& sortColumns,
                               const int columnOffset,
                               QHash<TrackId, int>* trackToIndex);
    virtual bool isCached(TrackId trackId) const;
    virtual void ensureCached(TrackId trackId);

  signals:
    void tracksChanged(const QSet<TrackId>& trackIds);

  public slots:
    void slotScanTrackAdded(TrackPointer pTrack);

    void slotTracksAddedOrChanged(const QSet<TrackId>& trackId);
    void slotTracksRemoved(const QSet<TrackId>& trackId);
    void slotTrackDirty(TrackId trackId);
    void slotTrackClean(TrackId trackId);

  private:
    const TrackPointer& getCachedTrack(TrackId trackId) const;
    void replaceRecentTrack(TrackPointer pTrack) const;
    void replaceRecentTrack(TrackId trackId, TrackPointer pTrack) const;
    void resetRecentTrack() const;

    bool updateIndexWithQuery(const QString& query);
    void updateTrackInIndex(TrackId trackId);
    bool updateTrackInIndex(const TrackPointer& pTrack);
    void updateTracksInIndex(const QSet<TrackId>& trackIds);
    QVariant getTrackValueForColumn(TrackPointer pTrack, int column) const;

    int findSortInsertionPoint(TrackPointer pTrack,
                               const QList<SortColumn>& sortColumns,
                               const int columnOffset,
                               const QVector<TrackId>& trackIds) const;
    int compareColumnValues(int sortColumn,
            Qt::SortOrder sortOrder,
            const QVariant& val1,
            const QVariant& val2) const;
    bool trackMatches(const TrackPointer& pTrack,
            const QRegularExpression& matcher) const;
    bool trackMatchesNumeric(const TrackPointer& pTrack,
                             const QStringList& numberMatchers) const;
    bool trackMatchesNamedString(const TrackPointer& pTrack,
                             const QStringList& numberMatchers) const;
    bool evaluateNumeric(const int value, const QString& expression) const;

    const QString m_tableName;
    const QString m_idColumn;
    const int m_columnCount;
    const QString m_columnsJoined;

    const ColumnCache m_columnCache;

    const std::unique_ptr<SearchQueryParser> m_pQueryParser;

    const mixxx::StringCollator m_collator;

    // Temporary storage for filterAndSort()

    QVector<TrackId> m_trackOrder;

    // Remember key and value of the most recent cache lookup to avoid querying
    // the global track cache again and again while populating the columns
    // of a single row. These members serve as a single-valued private cache.
    mutable TrackId m_recentTrackId;
    mutable TrackPointer m_recentTrackPtr;

    // This set is updated by signals from the Track object. It might contain
    // false positives, i.e. track ids of tracks that are neither cached nor
    // dirty. Each invocation of getRecentTrack() will take care of updating
    // this set by inserting and removing entries as required.
    mutable QSet<TrackId> m_dirtyTracks;

    bool m_bIndexBuilt;
    bool m_bIsCaching;
    QHash<TrackId, QVector<QVariant>> m_trackInfo;
    QSqlDatabase m_database;

    DISALLOW_COPY_AND_ASSIGN(BaseTrackCache);
};
