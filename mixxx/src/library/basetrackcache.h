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

#include "trackinfoobject.h"
#include "library/dao/trackdao.h"

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
    BaseTrackCache(QObject* pParent,
                   TrackCollection* pTrackCollection,
                   QSqlDatabase db,
                   QString tableName,
                   QString idColumn,
                   QList<QString> columns);
    virtual ~BaseTrackCache();

    ////////////////////////////////////////////////////////////////////////////
    // Data access methods
    ////////////////////////////////////////////////////////////////////////////

    virtual QVariant data(int trackId, QString column) const;
    virtual const QStringList columns() const;
    virtual int columnCount() const;
    virtual void filterAndSort(QVector<int>& trackIds, QString query, QString extraFilter,
                               QString sortColumn, Qt::SortOrder sortOrder) const;
    virtual void ensureCached(QSet<int> trackIds);

  signals:
    void tracksChanged(QSet<int> trackIds);

  private slots:
    void slotTrackAdded(int trackId);
    void slotTrackRemoved(int trackId);
    void slotTrackDirty(int trackId);
    void slotTrackClean(int trackId);

  private:
    int fieldIndex(const QString& fieldName) const;
    TrackPointer lookupCachedTrack(int trackId) const;
    bool updateIndexWithQuery(QString query);
    void buildIndex();
    void updateTrackInIndex(int trackId);
    void updateTracksInIndex(QSet<int> trackIds);
    QVariant getTrackValueForColumn(TrackPointer pTrack, QString column) const;

    QString filterClause(QString query, QString extraFilter,
                         QStringList idStrings) const;
    QString orderByClause(QString sortColumn, Qt::SortOrder sortOrder) const;
    int findSortInsertionPoint(TrackPointer pTrack,
                               const QString sortColumn,
                               const Qt::SortOrder sortOrder,
                               const QVector<int> trackIds) const;
    int compareColumnValues(QString sortColumn, Qt::SortOrder sortOrder,
                            QVariant val1, QVariant val2) const;

    const QString m_tableName;
    const QString m_idColumn;
    const QStringList m_columns;
    const QString m_columnsJoined;
    const QHash<QString, int> m_columnIndex;

    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;

    QSet<int> m_dirtyTracks;

    bool m_bIndexBuilt;
    QHash<int, QVector<QVariant> > m_trackInfo;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
    QSqlDatabase m_database;
};

#endif // BASETRACKCACHE_H
