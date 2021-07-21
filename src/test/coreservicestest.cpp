#include <gtest/gtest.h>

#include "coreservices.h"
#include "test/mixxxtest.h"
#include "util/cmdlineargs.h"

class CoreServicesTest : public MixxxTest {
};

TEST_F(CoreServicesTest, TestInitialization) {
    // Initialize the app
    const int argc = 0;
    CmdlineArgs cmdlineArgs;
    cmdlineArgs.parse(argc, nullptr);

    const auto pCoreServices = std::make_unique<mixxx::CoreServices>(cmdlineArgs);
    pCoreServices->initializeSettings();
    pCoreServices->initializeKeyboard();
    pCoreServices->initialize(application());

    EXPECT_NE(pCoreServices->getControllerManager(), nullptr);
    EXPECT_NE(pCoreServices->getEffectsManager(), nullptr);
    EXPECT_NE(pCoreServices->getGuiTick(), nullptr);
    EXPECT_NE(pCoreServices->getLV2Backend(), nullptr);
    EXPECT_NE(pCoreServices->getLibrary(), nullptr);
    EXPECT_NE(pCoreServices->getPlayerManager(), nullptr);
    EXPECT_NE(pCoreServices->getScreensaverManager(), nullptr);
    EXPECT_NE(pCoreServices->getSettingsManager(), nullptr);
    EXPECT_NE(pCoreServices->getSoundManager(), nullptr);
    EXPECT_NE(pCoreServices->getVinylControlManager(), nullptr);
    EXPECT_NE(pCoreServices->getVisualsManager(), nullptr);
}
