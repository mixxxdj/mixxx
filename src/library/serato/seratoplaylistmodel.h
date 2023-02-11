#pragma once
// seratoplaylistmodel.h
// Created 2020-02-15 by Jan Holthuis
#include "library/baseexternalplaylistmodel.h"

class TrackCollectionManager;
class BaseExternalPlaylistModel;

class SeratoPlaylistModel : public BaseExternalPlaylistModel {
    Q_OBJECT
  public:
    SeratoPlaylistModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);
    TrackPointer getTrack(const QModelIndex& index) const override;
    bool isColumnHiddenByDefault(int column) override;

  protected:
    void initSortColumnMapping() override;
};
