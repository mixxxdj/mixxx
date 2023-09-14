#include <gtest/gtest.h>

#include "coreservices.h"
#include "test/mixxxtest.h"

class CoreServicesTest : public MixxxTest {
};

// This test is disabled because it fails on CI for some unknown reason.
TEST_F(CoreServicesTest, TestInitialization) {
    const auto pCoreServices = std::make_unique<mixxx::CoreServices>(cmdLineArgs(), application());
    pCoreServices->initialize(application());

    EXPECT_NE(pCoreServices->getControllerManager(), nullptr);
    EXPECT_NE(pCoreServices->getEffectsManager(), nullptr);
    EXPECT_NE(pCoreServices->getLibrary(), nullptr);
    EXPECT_NE(pCoreServices->getPlayerManager(), nullptr);
    EXPECT_NE(pCoreServices->getScreensaverManager(), nullptr);
    EXPECT_NE(pCoreServices->getSettingsManager(), nullptr);
    EXPECT_NE(pCoreServices->getSoundManager(), nullptr);
    EXPECT_NE(pCoreServices->getVinylControlManager(), nullptr);
}
