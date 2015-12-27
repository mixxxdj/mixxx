#include "test/mixxxtest.h"

#include "soundsourceproxy.h"

#ifdef __FFMPEGFILE__
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#endif

// Static initialization
QScopedPointer<MixxxApplication> MixxxTest::s_pApplication;

MixxxTest::ApplicationScope::ApplicationScope(int argc, char** argv) {
    DEBUG_ASSERT(!s_pApplication);

#ifdef __FFMPEGFILE__
    av_register_all();
    avcodec_register_all();
#endif

    s_pApplication.reset(new MixxxApplication(argc, argv));

    SoundSourceProxy::loadPlugins();
}

MixxxTest::ApplicationScope::~ApplicationScope() {
    DEBUG_ASSERT(s_pApplication);

    s_pApplication.reset();
}

MixxxTest::MixxxTest()
    // This directory has to be deleted later to clean up the test env.
    : m_testDataDir(QDir::current().absoluteFilePath("src/test/test_data")),
      m_testDataCfg(m_testDataDir.filePath("test.cfg")),
      m_pConfig(new ConfigObject<ConfigValue>(m_testDataCfg)) {
}

namespace {

bool QDir_removeRecursively(const QDir& dir) {
    bool result = true;
    if (dir.exists()) {
        qDebug() << "dir exists" << dir;
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

}

MixxxTest::~MixxxTest() {
    // Mixxx leaks a ton of COs normally. To make new tests not affected by
    // previous tests, we clear our all COs after every MixxxTest completion.
    QList<QSharedPointer<ControlDoublePrivate> > leakedControls;
    ControlDoublePrivate::getControls(&leakedControls);
    foreach (QSharedPointer<ControlDoublePrivate> pCDP, leakedControls) {
        if (pCDP.isNull()) {
            continue;
        }
        ConfigKey key = pCDP->getKey();
        qDebug() << "Warning: Test leaked control:" << key.group << key.item;
        delete pCDP->getCreatorCO();
    }

    // recursivly delete all config files used for the test.
    // TODO(kain88) --
    //     switch to use QDir::removeRecursively() once we switched to Qt5.
    QDir_removeRecursively(m_testDataDir);
}
