#pragma once

#include <QObject>

#include "library/baseexternaltrackmodel.h"

class ITunesTrackModel : public BaseExternalTrackModel {
    Q_OBJECT
  public:
    ITunesTrackModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);
};
