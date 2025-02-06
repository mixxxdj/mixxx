#pragma once

#include "library/baseexternalplaylistmodel.h"

class TrackCollectionManager;
class SeratoPlaylistModel;

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
