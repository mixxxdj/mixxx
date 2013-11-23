#include <gtest/gtest.h>
#include <QDebug>

#include "controlobject.h"

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
    co1->set(1.0f);
    EXPECT_DOUBLE_EQ(1.0f, co1->get());
    co2->set(2.0f);
    EXPECT_DOUBLE_EQ(2.0f, co2->get());
}

TEST_F(ControlObjectTest, getControl) {
    EXPECT_EQ(ControlObject::getControl(ck1), co1);
    EXPECT_EQ(ControlObject::getControl(ck2), co2);
    delete co2;
    co2 = NULL;
    EXPECT_EQ(ControlObject::getControl(ck2), (ControlObject*)NULL);
}

TEST_F(ControlObjectTest, connectControls) {
    ControlObject::connectControls(ck1, ck2);
    co1->set(1.0f);
    EXPECT_DOUBLE_EQ(1.0f, co1->get());
    EXPECT_DOUBLE_EQ(1.0f, co2->get());
    ControlObject::disconnectControl(ck1);
    co1->set(2.0f);
    EXPECT_DOUBLE_EQ(2.0f, co1->get());
    EXPECT_DOUBLE_EQ(1.0f, co2->get());
}

TEST_F(ControlObjectTest, syncDoesntCrash) {
    co1->set(1.0f);
    co2->set(2.0f);
    ControlObject::sync();
    co1->set(0.0f);
    co2->set(1.0f);
    delete co2;
    co2 = NULL;
    ControlObject::sync();
    co1->set(0.0f);
    delete co1;
    co1 = NULL;
    ControlObject::sync();
}

}
