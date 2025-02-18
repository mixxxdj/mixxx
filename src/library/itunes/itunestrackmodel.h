#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "library/baseexternaltrackmodel.h"
#include "library/basetrackcache.h"
#include "library/trackcollectionmanager.h"

class ITunesTrackModel : public BaseExternalTrackModel {
    Q_OBJECT
  public:
    ITunesTrackModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);

  protected:
    QString resolveLocation(const QString& nativeLocation) const override;
};
