#ifndef MIXXXTEST_H
#define MIXXXTEST_H

#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QScopedPointer>

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

  private:
    bool removeDir(const QString& dirName);
    QString testDataDir;
};


#endif /* MIXXXTEST_H */
