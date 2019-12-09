#pragma once

#include <QList>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QVector>

#include <memory>

#include "library/columncache.h"
#include "track/track.h"
#include "util/class.h"
#include "util/string.h"

class SearchQueryParser;
class TrackDAO;
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
    BaseTrackCache(TrackCollection* pTrackCollection,
                   const QString& tableName,
                   const QString& idColumn,
                   const QStringList& columns,
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
    virtual void ensureCached(QSet<TrackId> trackIds);
    virtual void setSearchColumns(const QStringList& columns);

  signals:
    void tracksChanged(QSet<TrackId> trackIds);

  public slots:
    void slotTracksAdded(QSet<TrackId> trackId);
    void slotTracksRemoved(QSet<TrackId> trackId);
    void slotTrackDirty(TrackId trackId);
    void slotTrackClean(TrackId trackId);
    void slotTrackChanged(TrackId trackId);
    void slotDbTrackAdded(TrackPointer pTrack);

  private:
    friend class TrackCollection;
    void connectTrackDAO(TrackDAO* pTrackDAO);
    void disconnectTrackDAO(TrackDAO* pTrackDAO);

    const TrackPointer& getRecentTrack(TrackId trackId) const;
    void replaceRecentTrack(TrackPointer pTrack) const;
    void replaceRecentTrack(TrackId trackId, TrackPointer pTrack) const;
    void resetRecentTrack() const;

    bool updateIndexWithQuery(const QString& query);
    bool updateIndexWithTrackpointer(TrackPointer pTrack);
    void updateTrackInIndex(TrackId trackId);
    void updateTracksInIndex(const QSet<TrackId>& trackIds);
    void getTrackValueForColumn(TrackPointer pTrack, int column,
                                QVariant& trackValue) const;

    int findSortInsertionPoint(TrackPointer pTrack,
                               const QList<SortColumn>& sortColumns,
                               const int columnOffset,
                               const QVector<TrackId>& trackIds) const;
    int compareColumnValues(int sortColumn, Qt::SortOrder sortOrder,
                            QVariant val1, QVariant val2) const;
    bool trackMatches(const TrackPointer& pTrack,
                      const QRegExp& matcher) const;
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

    const StringCollator m_collator;

    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;

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
    QHash<TrackId, QVector<QVariant> > m_trackInfo;
    QSqlDatabase m_database;
    ControlProxy* m_pKeyNotationCP;

    DISALLOW_COPY_AND_ASSIGN(BaseTrackCache);
};
