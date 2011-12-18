#ifndef MIXXXTEST_H
#define MIXXXTEST_H

#include <gtest/gtest.h>

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"

class MixxxTest : public testing::Test {
  public:
    MixxxTest() {
        m_pConfig.reset(new ConfigObject<ConfigValue>(""));
    }

  protected:
    ControlObjectThread* getControlObjectThread(ConfigKey key) {
        return new ControlObjectThread(ControlObject::getControl(key));
    }
    QScopedPointer<ConfigObject<ConfigValue> > m_pConfig;
};


#endif /* MIXXXTEST_H */
