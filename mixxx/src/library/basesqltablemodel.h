// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QtCore>
#include <QHash>
#include <QtGui>
#include <QtSql>

#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"

#include "util.h"

// BaseSqlTableModel is a custom-written SQL-backed table which aggressively
// caches the contents of the table and supports lightweight updates.
class BaseSqlTableModel : public QAbstractTableModel, public TrackModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(QObject* pParent,
                      TrackCollection* pTrackCollection,
                      QSqlDatabase db, QString settingsNamespace);
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
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

    ////////////////////////////////////////////////////////////////////////////
    // Other public methods
    ////////////////////////////////////////////////////////////////////////////

    virtual void search(const QString& searchText, const QString extraFilter=QString());
    virtual void setSearch(const QString& searchText, const QString extraFilter=QString());
    virtual const QString currentSearch() const;
    virtual void setSort(int column, Qt::SortOrder order);
    virtual int fieldIndex(const QString& fieldName) const;
    virtual void select();
    virtual int getTrackId(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);
    virtual void hideTracks(const QModelIndexList& indices);

  protected:
    // Returns the row of trackId in this result set. If trackId is not present,
    // returns -1.
    virtual const QLinkedList<int> getTrackRows(int trackId) const;

    virtual void setTable(const QString& tableName,
                          const QString& trackIdColumn,
                          const QStringList& tableColumns,
                          QSharedPointer<BaseTrackCache> trackSource);
    QSqlDatabase database() const;

    /** Use this if you want a model that is read-only. */
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    /** Use this if you want a model that can be changed  */
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;
    /** calls readWriteFlags() by default */
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    virtual QString orderByClause() const;
    virtual void initHeaderData();

  private slots:
    void tracksChanged(QSet<int> trackIds);

  private:
    inline void setTrackValueForColumn(TrackPointer pTrack, int column, QVariant value);
    QVariant getBaseValue(const QModelIndex& index, int role = Qt::DisplayRole) const;

    struct RowInfo {
        int trackId;
        int order;
        QHash<int, QVariant> metadata;

        bool operator<(const RowInfo& other) const {
            // -1 is greater than anything
            if (order == -1) {
                return false;
            } else if (other.order == -1) {
                return true;
            }
            return order < other.order;
        }
    };

    QString m_tableName;
    QString m_idColumn;
    QSharedPointer<BaseTrackCache> m_trackSource;
    QStringList m_tableColumns;
    QString m_tableColumnsJoined;
    QHash<QString, int> m_tableColumnIndex;

    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;

    bool m_bInitialized;
    bool m_bDirty;
    QSqlRecord m_queryRecord;
    QVector<RowInfo> m_rowInfo;
    QHash<int, int> m_trackSortOrder;
    QHash<int, QLinkedList<int> > m_trackIdToRows;

    QString m_currentSearch;
    QString m_currentSearchFilter;

    QVector<QHash<int, QVariant> > m_headerInfo;

    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
    QSqlDatabase m_database;

    DISALLOW_COPY_AND_ASSIGN(BaseSqlTableModel);
};

#endif /* BASESQLTABLEMODEL_H */
