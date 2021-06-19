#pragma once

#include "library/basesqltablemodel.h"

class HiddenTableModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~HiddenTableModel() final;

    void setTableModel();

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    void unhideTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    Capabilities getCapabilities() const final;
};
