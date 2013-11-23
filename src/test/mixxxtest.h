#ifndef MIXXXTEST_H
#define MIXXXTEST_H

#include <gtest/gtest.h>

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
        m_pConfig.reset(new ConfigObject<ConfigValue>(""));
    }

  protected:
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

    QScopedPointer<ConfigObject<ConfigValue> > m_pConfig;
};


#endif /* MIXXXTEST_H */
