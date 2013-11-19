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
                      QString settingsNamespace);
    virtual ~BaseSqlTableModel();

    ///////////////////////////////////////////////////////////////////////////
    //  Functions that have to/can be reimplemented
    ///////////////////////////////////////////////////////////////////////////
    //  This class also has protected variables that should be used in childs
    //  m_database, m_pTrackCollection, m_trackDAO

    virtual void setTableModel(int id=-1) = 0;
    virtual bool isColumnInternal(int column) = 0;
    virtual bool isColumnHiddenByDefault(int column) = 0;
    virtual TrackModel::CapabilitiesFlags getCapabilities() const = 0;

    // functions that can be implemented
    // function to reimplement for external libraries
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    // calls readWriteFlags() by default, reimplement this if the child calls
    // should be readOnly
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    ////////////////////////////////////////////////////////////////////////////
    // Other public methods
    ////////////////////////////////////////////////////////////////////////////

    // Returns true if the BaseSqlTableModel has been initialized. Calling data
    // access methods on a BaseSqlTableModel which is not initialized is likely
    // to cause instability / crashes.
    bool initialized() const { return m_bInitialized; }
    int getTrackId(const QModelIndex& index) const;
    void search(const QString& searchText, const QString& extraFilter=QString());
    void setSearch(const QString& searchText, const QString extraFilter=QString());
    const QString currentSearch() const;
    void setSort(int column, Qt::SortOrder order);
    void hideTracks(const QModelIndexList& indices);
    int fieldIndex(const QString& fieldName) const;
    void select();
    QString getTrackLocation(const QModelIndex& index) const;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);

    // Methods reimplemented from QAbstractItemModel
    void sort(int column, Qt::SortOrder order);
    int rowCount(const QModelIndex& parent=QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    int columnCount(const QModelIndex& parent=QModelIndex()) const;
    bool setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant &value, int role=Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                                int role=Qt::DisplayRole) const;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

  protected:
    // Returns the row of trackId in this result set. If trackId is not present,
    // returns -1.
    const QLinkedList<int> getTrackRows(int trackId) const;
    void setTable(const QString& tableName, const QString& trackIdColumn,
                  const QStringList& tableColumns,
                  QSharedPointer<BaseTrackCache> trackSource);
    void initHeaderData();

    // Use this if you want a model that is read-only.
    Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    // Use this if you want a model that can be changed
    Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
    QSqlDatabase m_database;

  private slots:
    void tracksChanged(QSet<int> trackIds);
    void trackLoaded(QString group, TrackPointer pTrack);

  private:
    inline void setTrackValueForColumn(TrackPointer pTrack, int column, QVariant value);
    QVariant getBaseValue(const QModelIndex& index, int role = Qt::DisplayRole) const;
    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    QString orderByClause() const;
    QSqlDatabase database() const;

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
    QVector<RowInfo> m_rowInfo;

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
    QHash<int, int> m_trackSortOrder;
    QHash<int, QLinkedList<int> > m_trackIdToRows;
    QString m_currentSearch;
    QString m_currentSearchFilter;
    QString m_previewDeckGroup;
    int m_iPreviewDeckTrackId;
    QVector<QHash<int, QVariant> > m_headerInfo;

    DISALLOW_COPY_AND_ASSIGN(BaseSqlTableModel);
};

#endif /* BASESQLTABLEMODEL_H */
