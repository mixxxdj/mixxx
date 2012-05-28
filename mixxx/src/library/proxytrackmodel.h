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
class ProxyTrackModel : public QSortFilterProxyModel, public virtual TrackModel {
  public:
    // Construct a new ProxyTrackModel with pTrackModel as the TrackModel it
    // composes. If bHandleSearches is true, then search signals will not be
    // delivered to pTrackModel -- instead the ProxyTrackModel will do its own
    // filtering.
    ProxyTrackModel(QAbstractItemModel* pTrackModel, bool bHandleSearches=true);
    virtual ~ProxyTrackModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual int getTrackId(const QModelIndex& index) const;
    virtual const QLinkedList<int> getTrackRows(int trackId) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch() const;
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    virtual QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

    virtual QString getModelSetting(QString name);
    virtual bool setModelSetting(QString name, QVariant value);

  private:
    TrackModel* m_pTrackModel;
    QString m_currentSearch;
    bool m_bHandleSearches;
};

#endif /* PROXYTRACKMODEL_H */
