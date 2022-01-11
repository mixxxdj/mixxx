#include "test/mixxxtest.h"

#include <QTemporaryFile>

#include "control/control.h"
#include "library/coverartutils.h"
#include "util/cmdlineargs.h"
#include "util/logging.h"

namespace {

QString makeTestConfigFile(const QString& path) {
    QFile test_cfg(path);
    test_cfg.open(QIODevice::ReadWrite);
    test_cfg.close();
    return path;
}

} // namespace

// Static initialization
QScopedPointer<MixxxApplication> MixxxTest::s_pApplication;

MixxxTest::ApplicationScope::ApplicationScope(int& argc, char** argv) {
    CmdlineArgs args;
    const bool argsParsed = args.parse(argc, argv);
    Q_UNUSED(argsParsed);
    DEBUG_ASSERT(argsParsed);

    mixxx::LogLevel logLevel = args.getLogLevel();

    // Log level Debug would produce too many log messages that
    // might abort and fail the CI builds.
    mixxx::Logging::initialize(
            QString(), // No log file should be written during tests, only output to stderr
            logLevel,
            logLevel,
            mixxx::LogFlag::DebugAssertBreak);

    // All guessing of cover art should be done synchronously
    // in the same thread during tests to prevent test failures
    // due to timing issues.
    disableConcurrentGuessingOfTrackCoverInfoDuringTests();

    DEBUG_ASSERT(s_pApplication.isNull());
    s_pApplication.reset(new MixxxApplication(argc, argv));
}

MixxxTest::ApplicationScope::~ApplicationScope() {
    DEBUG_ASSERT(!s_pApplication.isNull());
    s_pApplication.reset();
    mixxx::Logging::shutdown();
}

MixxxTest::MixxxTest() {
    DEBUG_ASSERT(m_testDataDir.isValid());
    m_pConfig = UserSettingsPointer(new UserSettings(
            makeTestConfigFile(getTestDataDir().filePath("test.cfg"))));
    ControlDoublePrivate::setUserConfig(m_pConfig);
}

MixxxTest::~MixxxTest() {
    // Mixxx leaks a ton of COs normally. To make new tests not affected by
    // previous tests, we clear our all COs after every MixxxTest completion.
    const auto controls = ControlDoublePrivate::takeAllInstances();
    for (auto pControl : controls) {
        pControl->deleteCreatorCO();
    }
}

void MixxxTest::saveAndReloadConfig() {
    m_pConfig->save();
    m_pConfig = UserSettingsPointer(
            new UserSettings(getTestDataDir().filePath("test.cfg")));
    ControlDoublePrivate::setUserConfig(m_pConfig);
}

namespace mixxxtest {

bool copyFile(const QString& srcFileName, const QString& dstFileName) {
    auto srcFile = QFile(srcFileName);
    DEBUG_ASSERT(srcFile.exists());
    VERIFY_OR_DEBUG_ASSERT(srcFile.copy(dstFileName)) {
        qWarning()
                << srcFile.errorString()
                << "- Failed to copy file"
                << srcFile.fileName()
                << "->"
                << dstFileName;
        return false;
    }
    auto dstFile = QFile(dstFileName);
    VERIFY_OR_DEBUG_ASSERT(dstFile.exists()) {
        return false;
    }
    if (srcFile.size() != dstFile.size()) {
        dstFile.remove();
        DEBUG_ASSERT(!"dstFile size does not match srcFile");
        return false;
    }

    return true;
}

} // namespace mixxxtest
