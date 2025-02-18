#pragma once

#include <QObject>

#include "library/baseexternalplaylistmodel.h"

class ITunesPlaylistModel : public BaseExternalPlaylistModel {
    Q_OBJECT
  public:
    ITunesPlaylistModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);

  protected:
    void initSortColumnMapping() override;
    QString resolveLocation(const QString& nativeLocation) const override;
};
