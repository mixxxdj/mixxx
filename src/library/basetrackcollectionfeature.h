#pragma once

#include "libraryfeature.h"

class BaseTrackCollectionFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BaseTrackCollectionFeature(Library * pLibrary,
                               UserSettingsPointer pConfig,
                               const QString& rootViewName);

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

};
