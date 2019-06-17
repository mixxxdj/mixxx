// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QHash>
#include <QtSql>

#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "library/columncache.h"
#include "util/class.h"

// BaseSqlTableModel is a custom-written SQL-backed table which aggressively
// caches the contents of the table and supports lightweight updates.
class BaseSqlTableModel : public QAbstractTableModel, public TrackModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(QObject* pParent,
                      TrackCollection* pTrackCollection,
                      const char* settingsNamespace);
    ~BaseSqlTableModel() override;

    // Returns true if the BaseSqlTableModel has been initialized. Calling data
    // access methods on a BaseSqlTableModel which is not initialized is likely
    // to cause instability / crashes.
    bool initialized() const {
        return m_bInitialized;
    }

    void setSearch(const QString& searchText, const QString& extraFilter = QString());
    void setSort(int column, Qt::SortOrder order);

    int fieldIndex(ColumnCache::Column column) const;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from TrackModel
    ///////////////////////////////////////////////////////////////////////////
    int fieldIndex(const QString& fieldName) const final;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////////////////////////
    void sort(int column, Qt::SortOrder order) final;
    int rowCount(const QModelIndex& parent=QModelIndex()) const final;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final;
    int columnCount(const QModelIndex& parent = QModelIndex()) const final;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::DisplayRole) final;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role=Qt::DisplayRole) const final;
    QMimeData* mimeData(const QModelIndexList &indexes) const final;

    ///////////////////////////////////////////////////////////////////////////
    //  Functions that might be reimplemented/overridden in derived classes
    ///////////////////////////////////////////////////////////////////////////
    //  This class also has protected variables that should be used in children
    //  m_database, m_pTrackCollection, m_trackDAO

    // calls readWriteFlags() by default, reimplement this if the child calls
    // should be readOnly
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from TrackModel
    ///////////////////////////////////////////////////////////////////////////
    bool isColumnHiddenByDefault(int column) override;
    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    const QLinkedList<int> getTrackRows(TrackId trackId) const override {
        return m_trackIdToRows.value(trackId);
    }
    QString getTrackLocation(const QModelIndex& index) const override;
    void hideTracks(const QModelIndexList& indices) override;
    void search(const QString& searchText, const QString& extraFilter = QString()) override;
    const QString currentSearch() const override;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) override;
    TrackModel::SortColumnId sortColumnIdFromColumnIndex(int column) override;
    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) override;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////////////////////////
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  public slots:
    void select();

  protected:
    void setTable(const QString& tableName, const QString& trackIdColumn,
                  const QStringList& tableColumns,
                  QSharedPointer<BaseTrackCache> trackSource);
    void initHeaderData();
    virtual void initSortColumnMapping();

    // Use this if you want a model that is read-only.
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    // Use this if you want a model that can be changed
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

    TrackCollection* m_pTrackCollection;
    QSqlDatabase m_database;

    QString m_previewDeckGroup;
    TrackId m_previewDeckTrackId;
    QString m_tableOrderBy;
    int m_columnIndexBySortColumnId[NUM_SORTCOLUMNIDS];
    QMap<int, TrackModel::SortColumnId> m_sortColumnIdByColumnIndex;

  private slots:
    virtual void tracksChanged(QSet<TrackId> trackIds);
    virtual void trackLoaded(QString group, TrackPointer pTrack);
    void refreshCell(int row, int column);

  private:
    // A simple helper function for initializing header title and width.  Note
    // that the ideal width of a column is based on the width of its data,
    // not the title string itself.
    void setHeaderProperties(ColumnCache::Column column, QString title, int defaultWidth);
    inline void setTrackValueForColumn(TrackPointer pTrack, int column, QVariant value);
    QVariant getBaseValue(const QModelIndex& index, int role = Qt::DisplayRole) const;
    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    QString orderByClause() const;

    struct RowInfo {
        TrackId trackId;
        int order;
        QVector<QVariant> metadata;

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

    typedef QHash<TrackId, QLinkedList<int>> TrackId2Rows;

    void clearRows();
    void replaceRows(
            QVector<RowInfo>&& rows,
            TrackId2Rows&& trackIdToRows);

    QVector<RowInfo> m_rowInfo;

    QString m_tableName;
    QString m_idColumn;
    QSharedPointer<BaseTrackCache> m_trackSource;
    QStringList m_tableColumns;
    ColumnCache m_tableColumnCache;
    QList<SortColumn> m_sortColumns;
    bool m_bInitialized;
    QHash<TrackId, int> m_trackSortOrder;
    TrackId2Rows m_trackIdToRows;
    QString m_currentSearch;
    QString m_currentSearchFilter;
    QVector<QHash<int, QVariant> > m_headerInfo;
    QString m_trackSourceOrderBy;

    DISALLOW_COPY_AND_ASSIGN(BaseSqlTableModel);
};

#endif /* BASESQLTABLEMODEL_H */
