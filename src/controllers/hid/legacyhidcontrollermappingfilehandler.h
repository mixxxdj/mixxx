#pragma once

#include <gtest/gtest_prod.h>

#include "controllers/legacycontrollermappingfilehandler.h"

class LegacyHidControllerMapping;

class LegacyHidControllerMappingFileHandler : public LegacyControllerMappingFileHandler {
  public:
    LegacyHidControllerMappingFileHandler(){};
    virtual ~LegacyHidControllerMappingFileHandler(){};

    bool save(const LegacyHidControllerMapping& mapping, const QString& fileName) const;

  private:
    virtual std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingsPath);

    FRIEND_TEST(LegacyControllerMappingFileHandlerTest, canSerializeMappingToFile);
};
