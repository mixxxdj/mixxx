#pragma once

#include <QSortFilterProxyModel>
#include <QAbstractItemModel>

#include "library/trackmodel.h"

// ProxyTrackModel composes a TrackModel inside of a QSortFilterProxyModel so
// that the sorting and filtering of the QSortFilterProxyModel can be completely
// transparent to the user of the TrackModel. The ProxyTrackModel will
// automatically translate any QModelIndex's to their source index before
// calling the composed TrackModel. If the bHandleSearches flag is set, the
// TrackModel search calls will not be delivered to the composed TrackModel
// because filtering is handled by the QSortFilterProxyModel.
class ProxyTrackModel : public QSortFilterProxyModel, public TrackModel {
  public:
    // Construct a new ProxyTrackModel with pTrackModel as the TrackModel it
    // composes. If bHandleSearches is true, then search signals will not be
    // delivered to pTrackModel -- instead the ProxyTrackModel will do its own
    // filtering.
    explicit ProxyTrackModel(QAbstractItemModel* pTrackModel, bool bHandleSearches = true);
    ~ProxyTrackModel() override;

    // Inherited from TrackModel
    Capabilities getCapabilities() const final;
    TrackPointer getTrack(const QModelIndex& index) const final;
    TrackPointer getTrackByRef(const TrackRef& trackRef) const final;
    QString getTrackLocation(const QModelIndex& index) const final;
    TrackId getTrackId(const QModelIndex& index) const final;
    CoverInfo getCoverInfo(const QModelIndex& index) const final;
    const QVector<int> getTrackRows(TrackId trackId) const final;
    void search(const QString& searchText,const QString& extraFilter = QString()) final;
    const QString currentSearch() const final;
    bool isColumnInternal(int column) final;
    bool isColumnHiddenByDefault(int column) final;
    void removeTracks(const QModelIndexList& indices) final;
    void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) final;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) final;
    QString getModelSetting(QString name) final;
    bool setModelSetting(QString name, QVariant value) final;
    TrackModel::SortColumnId sortColumnIdFromColumnIndex(int index) override;
    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) override;

    // Inherited from QSortFilterProxyModel
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const final;

    // Inherited from QAbstractItemModel
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) final;

  private:
    TrackModel* m_pTrackModel;
    QString m_currentSearch;
    bool m_bHandleSearches;
};
