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

FileRemover::~FileRemover() {
    VERIFY_OR_DEBUG_ASSERT(
            m_fileName.isEmpty() ||
            QFile::remove(m_fileName) ||
            !QFile::exists(m_fileName)) {
        // unexpected
    }
}

QString generateTemporaryFileName(const QString& fileNameTemplate) {
    auto tmpFile = QTemporaryFile(fileNameTemplate);
    // The file must be opened to create it and to obtain
    // its file name!
    VERIFY_OR_DEBUG_ASSERT(tmpFile.open()) {
        return QString();
    }
    const auto tmpFileName = tmpFile.fileName();
    DEBUG_ASSERT(!tmpFileName.isEmpty());
    // The empty temporary file will be removed upon returning
    // from this function
    return tmpFileName;
}

QString createEmptyTemporaryFile(const QString& fileNameTemplate) {
    auto emptyFile = QTemporaryFile(fileNameTemplate);
    VERIFY_OR_DEBUG_ASSERT(emptyFile.open()) {
        return QString();
    }

    // Retrieving the file's name after opening it is required to actually
    // create a named file on Linux.
    const auto fileName = emptyFile.fileName();
    DEBUG_ASSERT(!fileName.isEmpty());
    VERIFY_OR_DEBUG_ASSERT(emptyFile.exists()) {
        return QString();
    }
    VERIFY_OR_DEBUG_ASSERT(emptyFile.size() == 0) {
        return QString();
    }

    emptyFile.setAutoRemove(false);
    return fileName;
}

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
    auto dstFileRemover = FileRemover(dstFileName);
    auto dstFile = QFile(dstFileName);
    VERIFY_OR_DEBUG_ASSERT(dstFile.exists()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(srcFile.size() == dstFile.size()) {
        return false;
    }
    dstFileRemover.keepFile();
    return true;
}

} // namespace mixxxtest
