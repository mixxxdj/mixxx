#ifndef HIDDENTABLEMODEL_H
#define HIDDENTABLEMODEL_H

#include "library/basesqltablemodel.h"

class HiddenTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~HiddenTableModel();

    void setTableModel(int id = -1);
    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    void purgeTracks(const QModelIndexList& indices);
    void unhideTracks(const QModelIndexList& indices);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;
};

#endif
