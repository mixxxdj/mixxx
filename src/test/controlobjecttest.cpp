#include <gtest/gtest.h>
#include <QtDebug>

#include "control/controlobject.h"

namespace {

class ControlObjectTest : public testing::Test {
  protected:

    ControlObjectTest() {
        qDebug() << "ControlObjectTest()";
    }

    virtual void SetUp() {
        qDebug() << "SetUp";
        ck1 = ConfigKey("[Channel1]", "co1");
        ck2 = ConfigKey("[Channel1]", "co2");
        co1 = new ControlObject(ck1);
        co2 = new ControlObject(ck2);
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        if(co1) {
            qDebug() << "Deleting " << co1;
            delete co1;
            co1 = NULL;
        }
        if(co2) {
            qDebug() << "Deleting " << co2;
            delete co2;
            co2 = NULL;
        }
    }

    ConfigKey ck1, ck2;
    ControlObject *co1, *co2;

};

TEST_F(ControlObjectTest, setGet) {
    co1->set(1.0);
    EXPECT_DOUBLE_EQ(1.0, co1->get());
    co2->set(2.0);
    EXPECT_DOUBLE_EQ(2.0, co2->get());
}

TEST_F(ControlObjectTest, getControl) {
    EXPECT_EQ(ControlObject::getControl(ck1), co1);
    EXPECT_EQ(ControlObject::getControl(ck2), co2);
    delete co2;
    co2 = NULL;
    EXPECT_EQ(ControlObject::getControl(ck2), (ControlObject*)NULL);
}

TEST_F(ControlObjectTest, aliasRetrieval) {
    ConfigKey ck("[Microphone1]", "volume");
    ConfigKey ckAlias("[Microphone]", "volume");

    // Create the Control Object
    ControlObject* co = new ControlObject(ck);

    // Insert the alias before it is going to be used
    ControlDoublePrivate::insertAlias(ckAlias, ck);

    // Check if getControl on alias returns us the original ControlObject
    EXPECT_EQ(ControlObject::getControl(ckAlias), co);
}

}
