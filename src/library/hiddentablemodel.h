#ifndef HIDDENTABLEMODEL_H
#define HIDDENTABLEMODEL_H

#include <QModelIndexList>
#include <QString>
#include <QtCore>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class TrackCollectionManager;

class HiddenTableModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~HiddenTableModel() final;

    void setTableModel(int id = -1);

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    void unhideTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;
};

#endif
