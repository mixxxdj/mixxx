#pragma once

#include <QObject>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class QModelIndex;
typedef QList<QModelIndex> QModelIndexList;

class MusicBrainzQueueTableModel final : public BaseSqlTableModel {
    Q_OBJECT

  public:
    MusicBrainzQueueTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~MusicBrainzQueueTableModel() final;

    void setTableModel();

    bool isColumnInternal(int column) final;
    Qt::ItemFlags flags(const QModelIndex& index) const final;
    Capabilities getCapabilities() const final;

    QString modelKey(bool noSearch) const override;
};
