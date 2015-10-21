// proxytrackmodel.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef PROXYTRACKMODEL_H
#define PROXYTRACKMODEL_H

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
    ProxyTrackModel(QAbstractItemModel* pTrackModel, bool bHandleSearches=true);
    virtual ~ProxyTrackModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual TrackId getTrackId(const QModelIndex& index) const;
    virtual const QLinkedList<int> getTrackRows(TrackId trackId) const;
    virtual void search(const QString& searchText,const QString& extraFilter=QString());
    virtual const QString currentSearch() const;
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    void deleteTracks(const QModelIndexList& indices);
    virtual QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

    virtual QString getModelSetting(QString name);
    virtual bool setModelSetting(QString name, QVariant value);

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  private:
    TrackModel* m_pTrackModel;
    QString m_currentSearch;
    bool m_bHandleSearches;
};

#endif /* PROXYTRACKMODEL_H */
