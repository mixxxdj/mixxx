// trackcache.h
// Created 7/3/2011 by RJ Ryan (rryan@mit.edu)

#ifndef BASETRACKCACHE_H
#define BASETRACKCACHE_H

#include <QList>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QVector>

#include "library/dao/trackdao.h"
#include "library/columncache.h"
#include "track/track.h"
#include "util/class.h"
#include "util/memory.h"

class SearchQueryParser;
class QueryNode;
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
    virtual ~BaseTrackCache();

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

  private slots:
    void slotTracksAdded(QSet<TrackId> trackId);
    void slotTracksRemoved(QSet<TrackId> trackId);
    void slotTrackDirty(TrackId trackId);
    void slotTrackClean(TrackId trackId);
    void slotTrackChanged(TrackId trackId);
    void slotDbTrackAdded(TrackPointer pTrack);

  private:
    TrackPointer lookupCachedTrack(TrackId trackId) const;
    bool updateIndexWithQuery(const QString& query);
    bool updateIndexWithTrackpointer(TrackPointer pTrack);
    void updateTrackInIndex(TrackId trackId);
    void updateTracksInIndex(QSet<TrackId> trackIds);
    void getTrackValueForColumn(TrackPointer pTrack, int column,
                                QVariant& trackValue) const;

    std::unique_ptr<QueryNode> parseQuery(QString query, QString extraFilter,
                          QStringList idStrings) const;
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

    ColumnCache m_columnCache;

    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;

    // Temporary storage for filterAndSort()

    QVector<TrackId> m_trackOrder;

    QSet<TrackId> m_dirtyTracks;

    bool m_bIndexBuilt;
    bool m_bIsCaching;
    QHash<TrackId, QVector<QVariant> > m_trackInfo;
    TrackDAO& m_trackDAO;
    QSqlDatabase m_database;
    SearchQueryParser* m_pQueryParser;

    DISALLOW_COPY_AND_ASSIGN(BaseTrackCache);
};

#endif // BASETRACKCACHE_H
