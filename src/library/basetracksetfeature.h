#pragma once

#include "library/libraryfeature.h"

class BaseTrackSetFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BaseTrackSetFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& rootViewName);

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;
};
