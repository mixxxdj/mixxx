#pragma once

#include "libraryfeature.h"

class BaseTrackCollectionFeature : public LibraryFeature {
  public:
    BaseTrackCollectionFeature(Library * pLibrary,
                               UserSettingsPointer pConfig,
                               const QString& rootViewName);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

};
