#ifndef MIXXXTEST_H
#define MIXXXTEST_H

#include <gtest/gtest.h>

#include <QDir>
#include <QTemporaryFile>
#include <QScopedPointer>

#include "mixxxapplication.h"

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"

#define EXPECT_QSTRING_EQ(expected, test) EXPECT_STREQ(qPrintable(expected), qPrintable(test))
#define ASSERT_QSTRING_EQ(expected, test) ASSERT_STREQ(qPrintable(expected), qPrintable(test))

typedef QScopedPointer<QTemporaryFile> ScopedTemporaryFile;
typedef QScopedPointer<ControlObject> ScopedControl;

class MixxxTest : public testing::Test {
  public:
    MixxxTest();
    virtual ~MixxxTest();

    // ApplicationScope creates QApplication as a singleton and keeps
    // it alive during all tests. This prevents issues with creating
    // and destroying the QApplication multiple times in the same process.
    // http://stackoverflow.com/questions/14243858/qapplication-segfaults-in-googletest
    class ApplicationScope {
    public:
        ApplicationScope(int argc, char** argv);
        ~ApplicationScope();
    };
    friend class ApplicationScope;

  protected:
    static QApplication* application() {
        return s_pApplication.data();
    }

    ConfigObject<ConfigValue>* config() {
        return m_pConfig.data();
    }

    ControlObjectThread* getControlObjectThread(const ConfigKey& key) {
        return new ControlObjectThread(key);
    }

    QTemporaryFile* makeTemporaryFile(const QString contents) {
        QByteArray contentsBa = contents.toLocal8Bit();
        QTemporaryFile* file = new QTemporaryFile();
        file->open();
        file->write(contentsBa);
        file->close();
        return file;
    }

  private:
    static QScopedPointer<MixxxApplication> s_pApplication;

    const QDir m_testDataDir;
    const QString m_testDataCfg;

  protected:
    const QScopedPointer<ConfigObject<ConfigValue> > m_pConfig;

};


#endif /* MIXXXTEST_H */
