#pragma once
#include <QSharedPointer>

// seratoplaylistmodel.h
// Created 2020-02-15 by Jan Holthuis
#include "library/baseexternalplaylistmodel.h"
#include "track/track_decl.h"

class TrackCollectionManager;
class BaseExternalPlaylistModel;
class BaseTrackCache;
class QModelIndex;
class QObject;
template<class T>
class QSharedPointer;

class SeratoPlaylistModel : public BaseExternalPlaylistModel {
  public:
    SeratoPlaylistModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);
    TrackPointer getTrack(const QModelIndex& index) const override;
    bool isColumnHiddenByDefault(int column) override;

  protected:
    void initSortColumnMapping() override;
};
