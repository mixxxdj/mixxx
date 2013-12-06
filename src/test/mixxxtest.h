#ifndef MIXXXTEST_H
#define MIXXXTEST_H

#include <gtest/gtest.h>

#include <QDir>
#include <QTemporaryFile>
#include <QScopedPointer>

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"

typedef QScopedPointer<QTemporaryFile> ScopedTemporaryFile;
typedef QScopedPointer<ControlObject> ScopedControl;

class MixxxTest : public testing::Test {
  public:
    MixxxTest() {
        static int argc = 1;
        static char* argv[1] = { strdup("test") };
        // start the app without the GUI so that we can generate and
        // destroy it several times in one thread, see
        // http://stackoverflow.com/questions/14243858/qapplication-segfaults-in-googletest
        m_pApplication = new QApplication(argc, argv, false);
        m_pConfig.reset(new ConfigObject<ConfigValue>(
            QDir::currentPath().append("/src/test/test_data/test.cfg")));
    }
    virtual ~MixxxTest() {
        delete m_pApplication;
    }

  protected:
    ControlObjectThread* getControlObjectThread(const ConfigKey& key) {
        return new ControlObjectThread(key);
    }

    ConfigObject<ConfigValue>* config() {
        return m_pConfig.data();
    }

    QApplication* application() {
        return m_pApplication;
    }

    QTemporaryFile* makeTemporaryFile(const QString contents) {
        QByteArray contentsBa = contents.toLocal8Bit();
        QTemporaryFile* file = new QTemporaryFile();
        file->open();
        file->write(contentsBa);
        file->close();
        return file;
    }

    QApplication* m_pApplication;
    QScopedPointer<ConfigObject<ConfigValue> > m_pConfig;
};


#endif /* MIXXXTEST_H */
