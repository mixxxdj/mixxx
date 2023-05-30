#pragma once

#include <QObject>

#include "library/baseexternalplaylistmodel.h"
#include "library/basetrackcache.h"
#include "library/trackcollectionmanager.h"

class ITunesPlaylistModel : public BaseExternalPlaylistModel {
    Q_OBJECT
  public:
    ITunesPlaylistModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);

  protected:
    void initSortColumnMapping() override;
};
