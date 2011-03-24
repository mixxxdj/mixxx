// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QtCore>
#include <QHash>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/dao/trackdao.h"

// BaseSqlTableModel is a custom-written SQL-backed table which aggressively
// caches the contents of the table and supports lightweight updates.
class BaseSqlTableModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(QObject* pParent,
                      TrackCollection* pTrackCollection,
                      QSqlDatabase db);
    virtual ~BaseSqlTableModel();

    ////////////////////////////////////////////////////////////////////////////
    // Methods implemented from QAbstractItemModel
    ////////////////////////////////////////////////////////////////////////////

    virtual void sort(int column, Qt::SortOrder order);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant &value, int role=Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role=Qt::DisplayRole) const;

    ////////////////////////////////////////////////////////////////////////////
    // Other public methods
    ////////////////////////////////////////////////////////////////////////////

    virtual void search(const QString& searchText, const QString extraFilter=QString());
    virtual QString currentSearch() const;
    virtual void setSort(int column, Qt::SortOrder order);
    virtual int fieldIndex(const QString& fieldName) const;
    virtual void select();

  protected:
    // Returns the row of trackId in this result set. If trackId is not present,
    // returns -1.
    virtual int getTrackRow(int trackId) const;

    virtual void setTable(const QString& tableName, const QStringList& columnNames, const QString& idColumn);

    virtual void buildIndex();
    QSqlDatabase database() const;

    /** Use this if you want a model that is read-only. */
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    /** Use this if you want a model that can be changed  */
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;
    /** calls readWriteFlags() by default */
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void setLibraryPrefix(QString sPrefix);

    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    virtual void setSearchColumns(const QStringList& searchColumns);
    virtual QString orderByClause() const;
    virtual QString filterClause() const;
    virtual void initHeaderData();
    void setCaching(bool isActive);
    virtual void initDefaultSearchColumns();

    virtual void updateTrackInIndex(int trackId);
    virtual void updateTracksInIndex(QList<int> trackIds);

  private slots:
    void trackChanged(int trackId);
    void trackClean(int trackId);

  private:
    inline TrackPointer lookupCachedTrack(int trackId) const;
    inline QVariant getTrackValueForColumn(TrackPointer pTrack, int column) const;
    inline QVariant getTrackValueForColumn(int trackId, int column,
                                           TrackPointer pTrack=TrackPointer()) const;
    inline void setTrackValueForColumn(TrackPointer pTrack, int column, QVariant value);
    QVariant getBaseValue(const QModelIndex& index, int role = Qt::DisplayRole) const;

    virtual int compareColumnValues(int iColumnNumber, Qt::SortOrder eSortOrder, QVariant val1, QVariant val2);
    virtual int findSortInsertionPoint(int trackId, TrackPointer pTrack,
                                       const QVector<int>& rowToTrack);
    bool m_isCachedModel;
    QString m_tableName;
    QStringList m_columnNames;
    QString m_columnNamesJoined;
    QHash<QString, int> m_columnIndex;
    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;
    QString m_idColumn;

    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;

    bool m_bInitialized;
    bool m_bIndexBuilt;
    QSqlRecord m_queryRecord;
    QHash<int, QVector<QVariant> > m_recordCache;
    QVector<int> m_rowToTrackId;
    QHash<int, int> m_trackIdToRow;
    QSet<int> m_trackOverrides;

    QString m_currentSearch;
    QString m_currentSearchFilter;

    QVector<QHash<int, QVariant> > m_headerInfo;

    TrackCollection* m_pTrackCollection;
    QString m_sPrefix;
    // TrackDAO isn't very const-correct
    mutable TrackDAO& m_trackDAO;
    QSqlDatabase m_database;
};

#endif /* BASESQLTABLEMODEL_H */
