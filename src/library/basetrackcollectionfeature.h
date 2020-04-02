#ifndef MIXXX_BASETRACKCOLLECTIONFEATURE_H
#define MIXXX_BASETRACKCOLLECTIONFEATURE_H

#include "libraryfeature.h"

class BaseTrackCollectionFeature : public LibraryFeature {
  public:
    BaseTrackCollectionFeature(Library * pLibrary,
                               UserSettingsPointer pConfig);
};


#endif //MIXXX_BASETRACKCOLLECTIONFEATURE_H
