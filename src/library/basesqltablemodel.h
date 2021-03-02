#pragma once

#include <QHash>
#include <QtSql>

#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/basetracktablemodel.h"
#include "library/columncache.h"
#include "util/class.h"

class TrackCollectionManager;

// BaseSqlTableModel is a custom-written SQL-backed table which aggressively
// caches the contents of the table and supports lightweight updates.
class BaseSqlTableModel : public BaseTrackTableModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
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

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////////////////////////
    int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    void sort(int column, Qt::SortOrder order) final;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from TrackModel
    ///////////////////////////////////////////////////////////////////////////
    int fieldIndex(const QString& fieldName) const final;

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    QString getTrackLocation(const QModelIndex& index) const override;

    CoverInfo getCoverInfo(const QModelIndex& index) const override;

    const QVector<int> getTrackRows(TrackId trackId) const override {
        return m_trackIdToRows.value(trackId);
    }

    void search(const QString& searchText, const QString& extraFilter = QString()) override;
    const QString currentSearch() const override;

    TrackModel::SortColumnId sortColumnIdFromColumnIndex(int column) const override;
    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const override;

    void hideTracks(const QModelIndexList& indices) override;

    void select() override;

    ///////////////////////////////////////////////////////////////////////////
    // Inherited from BaseTrackTableModel
    ///////////////////////////////////////////////////////////////////////////
    int fieldIndex(
            ColumnCache::Column column) const final;

  protected:
    ///////////////////////////////////////////////////////////////////////////
    // Inherited from BaseTrackTableModel
    ///////////////////////////////////////////////////////////////////////////
    QVariant rawValue(
            const QModelIndex& index) const override;

    bool setTrackValueForColumn(
            const TrackPointer& pTrack,
            int column,
            const QVariant& value,
            int role) final;

    void setTable(const QString& tableName, const QString& trackIdColumn,
                  const QStringList& tableColumns,
                  QSharedPointer<BaseTrackCache> trackSource);
    void initHeaderProperties() override;
    virtual void initSortColumnMapping();

    TrackCollectionManager* const m_pTrackCollectionManager;

  protected:
    QList<TrackRef> getTrackRefs(const QModelIndexList& indices) const;

    QSqlDatabase m_database;

    QString m_tableOrderBy;
    int m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::IdMax)];
    QMap<int, TrackModel::SortColumnId> m_sortColumnIdByColumnIndex;

  private slots:
    void tracksChanged(const QSet<TrackId>& trackIds);

  private:
    void setTrackValueForColumn(
            TrackPointer pTrack, int column, QVariant value);

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

    typedef QHash<TrackId, QVector<int>> TrackId2Rows;

    void clearRows();
    void replaceRows(
            QVector<RowInfo>&& rows,
            TrackId2Rows&& trackIdToRows);

    QVector<RowInfo> m_rowInfo;

    QString m_tableName;
    QString m_idColumn;
    QSharedPointer<BaseTrackCache> m_trackSource;
    QStringList m_tableColumns;
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
