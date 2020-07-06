#include "test/mixxxtest.h"

#include "library/coverartutils.h"
#include "sources/soundsourceproxy.h"

namespace {

QString makeTestConfigFile(const QString& path) {
    QFile test_cfg(path);
    test_cfg.open(QIODevice::ReadWrite);
    test_cfg.close();
    return path;
}

}  // namespace

// Static initialization
QScopedPointer<MixxxApplication> MixxxTest::s_pApplication;

MixxxTest::ApplicationScope::ApplicationScope(int& argc, char** argv) {
    DEBUG_ASSERT(!s_pApplication);

    s_pApplication.reset(new MixxxApplication(argc, argv));

    SoundSourceProxy::registerSoundSourceProviders();

    // All guessing of cover art should be done synchronously
    // in the same thread during tests to prevent test failures
    // due to timing issues.
    disableConcurrentGuessingOfTrackCoverInfoDuringTests();
}

MixxxTest::ApplicationScope::~ApplicationScope() {
    DEBUG_ASSERT(s_pApplication);
    s_pApplication.reset();
}

MixxxTest::MixxxTest() {
    EXPECT_TRUE(m_testDataDir.isValid());
    m_pConfig = UserSettingsPointer(new UserSettings(
        makeTestConfigFile(getTestDataDir().filePath("test.cfg"))));
    ControlDoublePrivate::setUserConfig(m_pConfig);
}

MixxxTest::~MixxxTest() {
    // Mixxx leaks a ton of COs normally. To make new tests not affected by
    // previous tests, we clear our all COs after every MixxxTest completion.
    for (auto pControl : ControlDoublePrivate::takeAllInstances()) {
        pControl->deleteCreatorCO();
    }
}
