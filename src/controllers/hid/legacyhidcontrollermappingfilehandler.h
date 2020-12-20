#pragma once

#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"

class LegacyHidControllerMappingFileHandler : public LegacyControllerMappingFileHandler {
  public:
    LegacyHidControllerMappingFileHandler(){};
    virtual ~LegacyHidControllerMappingFileHandler(){};

    bool save(const LegacyHidControllerMapping& mapping, const QString& fileName) const;

  private:
    virtual LegacyControllerMappingPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingsPath);
};
