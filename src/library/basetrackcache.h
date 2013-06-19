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
#include "trackinfoobject.h"
#include "util.h"

class SearchQueryParser;
class TrackCollection;

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
                   QString tableName,
                   QString idColumn,
                   QList<QString> columns,
                   bool isCaching);
    virtual ~BaseTrackCache();

    // Rebuild the BaseTrackCache index from the SQL table. This can be
    // expensive on large tables.
    virtual void buildIndex();

    ////////////////////////////////////////////////////////////////////////////
    // Data access methods
    ////////////////////////////////////////////////////////////////////////////

    virtual QVariant data(int trackId, int column) const;
    virtual const QStringList columns() const;
    virtual int columnCount() const;
    virtual int fieldIndex(const QString column) const;
    virtual void filterAndSort(const QSet<int>& trackIds,
                               QString query, QString extraFilter,
                               int sortColumn, Qt::SortOrder sortOrder,
                               QHash<int, int>* trackToIndex);
    virtual bool isCached(int trackId) const;
    virtual void ensureCached(int trackId);
    virtual void ensureCached(QSet<int> trackIds);

  signals:
    void tracksChanged(QSet<int> trackIds);

  private slots:
    void slotTracksAdded(QSet<int> trackId);
    void slotTracksRemoved(QSet<int> trackId);
    void slotTrackDirty(int trackId);
    void slotTrackClean(int trackId);
    void slotTrackChanged(int trackId);
    void slotDbTrackAdded(TrackPointer pTrack);

  private:
    TrackPointer lookupCachedTrack(int trackId) const;
    bool updateIndexWithQuery(QString query);
    bool updateIndexWithTrackpointer(TrackPointer pTrack);
    void updateTrackInIndex(int trackId);
    void updateTracksInIndex(QSet<int> trackIds);
    void getTrackValueForColumn(TrackPointer pTrack, int column,
                                    QVariant& trackValue) const;

    QString filterClause(QString query, QString extraFilter,
                         QStringList idStrings) const;
    QString orderByClause(int sortColumn, Qt::SortOrder sortOrder) const;
    int findSortInsertionPoint(TrackPointer pTrack,
                               const int sortColumn,
                               const Qt::SortOrder sortOrder,
                               const QVector<int> trackIds) const;
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
    const QStringList m_columns;
    const QString m_columnsJoined;
    const QHash<QString, int> m_columnIndex;

    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;

    // Temporary storage for filterAndSort()

    QVector<int> m_trackOrder;

    QSet<int> m_dirtyTracks;

    bool m_bIndexBuilt;
    bool m_bIsCaching;
    QHash<int, QVector<QVariant> > m_trackInfo;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
    QSqlDatabase m_database;
    SearchQueryParser* m_pQueryParser;
    
    QStringList m_numericFilters;
    QRegExp m_operatorMatcher;
    QRegExp m_numericFilterMatcher;
    QRegExp m_stringFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(BaseTrackCache);
};

#endif // BASETRACKCACHE_H
