#ifndef HIDDENTABLEMODEL_H
#define HIDDENTABLEMODEL_H

#include "library/basesqltablemodel.h"

class HiddenTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~HiddenTableModel() final;

    void setTableModel(int id = -1);

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    void unhideTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;
};

#endif
