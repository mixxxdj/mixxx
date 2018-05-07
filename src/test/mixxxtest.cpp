#include "test/mixxxtest.h"

#include "sources/soundsourceproxy.h"

namespace {

bool QDir_removeRecursively(const QDir& dir) {
    bool result = true;
    if (dir.exists()) {
        foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot |
                                                   QDir::System |
                                                   QDir::Hidden  |
                                                   QDir::AllDirs |
                                                   QDir::Files,
                                                   QDir::DirsFirst)) {
            if (info.isDir()) {
                // recursively
                result = QDir_removeRecursively(QDir(info.absoluteFilePath()));
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dir.absolutePath());
    }
    return result;
}

QString makeTestDir() {
    QDir parent("src/test");
    parent.mkdir("test_data");
    return parent.absoluteFilePath("test_data");
}

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

    SoundSourceProxy::loadPlugins();
}

MixxxTest::ApplicationScope::~ApplicationScope() {
    DEBUG_ASSERT(s_pApplication);

    s_pApplication.reset();
}

MixxxTest::MixxxTest()
        // This directory has to be deleted later to clean up the test env.
        : m_testDataDir(makeTestDir()),
          m_pConfig(new UserSettings(makeTestConfigFile(
              m_testDataDir.filePath("test.cfg")))) {
    ControlDoublePrivate::setUserConfig(m_pConfig);
}

MixxxTest::~MixxxTest() {
    // Mixxx leaks a ton of COs normally. To make new tests not affected by
    // previous tests, we clear our all COs after every MixxxTest completion.
    QList<QSharedPointer<ControlDoublePrivate>> leakedControls;
    ControlDoublePrivate::getControls(&leakedControls);
    foreach (QSharedPointer<ControlDoublePrivate> pCDP, leakedControls) {
        if (pCDP.isNull()) {
            continue;
        }
        ConfigKey key = pCDP->getKey();
        delete pCDP->getCreatorCO();
    }

    // recursivly delete all config files used for the test.
    // TODO(kain88) --
    //     switch to use QDir::removeRecursively() once we switched to Qt5.
    QDir_removeRecursively(m_testDataDir);
}
