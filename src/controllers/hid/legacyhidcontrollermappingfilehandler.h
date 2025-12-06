#pragma once

#ifdef BUILD_TESTING
#include <gtest/gtest_prod.h>
#endif

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

#ifdef BUILD_TESTING
    FRIEND_TEST(LegacyControllerMappingFileHandlerTest, canSerializeMappingToFile);
#endif
};
