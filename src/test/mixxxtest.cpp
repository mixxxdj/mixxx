#include "test/mixxxtest.h"
#include "util/singleton.h"
<<<<<<< HEAD
#include "mixxxapplication.h"
<<<<<<< HEAD
=======
=======
#include "soundsourceproxy.h"
>>>>>>> Fix static initialization of SoundSource providers in tests

#ifdef __FFMPEGFILE__
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#endif

>>>>>>> Fix missing FFmpeg initialization in unit tests

// Specialize the Singleton template for QApplication because it doesn't have a
// 0-args constructor.
template <>
MixxxApplication* Singleton<MixxxApplication>::create() {
    if (!m_instance) {
        static int argc = 1;
        static char* argv[1] = { strdup("test") };
<<<<<<< HEAD
        m_instance = new MixxxApplication(argc, argv);
=======
#ifdef __FFMPEGFILE__
         av_register_all();
         avcodec_register_all();
#endif
        m_instance = new QApplication(argc, argv);
>>>>>>> Fix missing FFmpeg initialization in unit tests
    }
    return m_instance;
}

MixxxTest::MixxxTest() {
    // Create QApplication as a singleton. This prevents issues with creating
    // and destroying the QApplication multiple times in the same process.
    // http://stackoverflow.com/questions/14243858/qapplication-segfaults-in-googletest

    // This directory has to be deleted later to clean up the test env.
    testDataDir = QDir::currentPath().append("/src/test/test_data/");

    m_pApplication = Singleton<MixxxApplication>::create();
    m_pConfig.reset(new ConfigObject<ConfigValue>(testDataDir + "test.cfg"));

    SoundSourceProxy::loadPlugins();
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
    //     switch to use QDir::removeRecursivly() once we switched to Qt5.
    removeDir(testDataDir);
}

bool MixxxTest::removeDir(const QString& dirName) {
    bool result = true;
    QDir dir(dirName);

    if (dir.exists()) {
        qDebug() << "dir exists";
        foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot |
                                                   QDir::System |
                                                   QDir::Hidden  |
                                                   QDir::AllDirs |
                                                   QDir::Files,
                                                   QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}
