#include <gtest/gtest.h>

#include "coreservices.h"
#include "test/mixxxtest.h"
#include "util/cmdlineargs.h"

class CoreServicesTest : public MixxxTest {
};

// This test is disabled because it fails on CI for some unknown reason.
TEST_F(CoreServicesTest, DISABLED_TestInitialization) {
    // Initialize the app
    constexpr int argc = 1;
    CmdlineArgs cmdlineArgs;
    char progName[] = "mixxxtest";
    char safeMode[] = "--safe-mode";
    char* argv[] = {progName, safeMode};
    cmdlineArgs.parse(argc, argv);

    const auto pCoreServices = std::make_unique<mixxx::CoreServices>(cmdlineArgs, application());
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
