#pragma once

#include "library/basesqltablemodel.h"

class TrackSetTableModel : public BaseSqlTableModel {
    Q_OBJECT

  public:
    TrackSetTableModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            const char* settingsNamespace);

    bool isColumnInternal(int column) override;

    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
};
