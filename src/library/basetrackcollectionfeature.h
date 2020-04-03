#ifndef MIXXX_BASETRACKCOLLECTIONFEATURE_H
#define MIXXX_BASETRACKCOLLECTIONFEATURE_H

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


#endif //MIXXX_BASETRACKCOLLECTIONFEATURE_H
