// proxytrackmodel.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef PROXYTRACKMODEL_H
#define PROXYTRACKMODEL_H

#include <QSortFilterProxyModel>

#include "library/trackmodel.h"

// ProxyTrackModel composes a TrackModel inside of a QSortFilterProxyModel so
// that the sorting and filtering of the QSortFilterProxyModel can be completely
// transparent to the user of the TrackModel. The ProxyTrackModel will
// automatically translate any QModelIndex's to their source index before
// calling the composed TrackModel. The TrackModel search signals will NOT be
// delivered to the composed TrackModel because filtering is handled by the
// QSortFilterProxyModel.
class ProxyTrackModel : public QSortFilterProxyModel, public virtual TrackModel {
  public:
    ProxyTrackModel(TrackModel* pTrackModel);
    virtual ~ProxyTrackModel();

    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    virtual QItemDelegate* delegateForColumn(const int i);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;
  private:
    TrackModel* m_pTrackModel;
    QString m_currentSearch;
};

#endif /* PROXYTRACKMODEL_H */
