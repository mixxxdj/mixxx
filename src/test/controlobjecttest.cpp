#include <gtest/gtest.h>
#include <QtDebug>

#include "control/controlobject.h"
#include "util/memory.h"
#include "test/mixxxtest.h"

namespace {

class ControlObjectTest : public MixxxTest {
  protected:
    void SetUp() override {
        ck1 = ConfigKey("[Channel1]", "co1");
        ck2 = ConfigKey("[Channel1]", "co2");
        co1 = std::make_unique<ControlObject>(ck1);
        co2 = std::make_unique<ControlObject>(ck2);
    }

    ConfigKey ck1, ck2;
    std::unique_ptr<ControlObject> co1;
    std::unique_ptr<ControlObject> co2;
};

TEST_F(ControlObjectTest, SetGet) {
    co1->set(1.0);
    EXPECT_DOUBLE_EQ(1.0, co1->get());
    co2->set(2.0);
    EXPECT_DOUBLE_EQ(2.0, co2->get());
}

TEST_F(ControlObjectTest, getControl) {
    EXPECT_EQ(ControlObject::getControl(ck1), co1.get());
    EXPECT_EQ(ControlObject::getControl(ck2), co2.get());
    co2.reset();
    EXPECT_EQ(ControlObject::getControl(ck2, ControlFlag::NoAssertIfMissing),
            (ControlObject*)nullptr);
}

TEST_F(ControlObjectTest, AliasRetrieval) {
    ConfigKey ck("[Microphone1]", "volume");
    ConfigKey ckAlias("[Microphone]", "volume");

    // Create the Control Object
    auto co = std::make_unique<ControlObject>(ck);

    // Insert the alias before it is going to be used
    ControlDoublePrivate::insertAlias(ckAlias, ck);

    // Check if getControl on alias returns us the original ControlObject
    EXPECT_EQ(ControlObject::getControl(ckAlias), co.get());
}

TEST_F(ControlObjectTest, Persistence_NotPresent) {
    ConfigKey ck("[Test]", "persist");
    ASSERT_FALSE(m_pConfig->exists(ck));
    ControlObject co(ck, true, false, true, 3.0);
    // Should be initialized to default value with no valid value in config
    EXPECT_DOUBLE_EQ(3.0, co.get());
}

TEST_F(ControlObjectTest, Persistence_InvalidValue) {
    ConfigKey ck("[Test]", "persist");
    m_pConfig->set(ck, QString("NotANumber"));
    saveAndReloadConfig();

    ControlObject co(ck, true, false, true, 3.0);
    EXPECT_DOUBLE_EQ(3.0, co.get());
}

TEST_F(ControlObjectTest, Persistence_EmptyValue) {
    ConfigKey ck("[Test]", "persist");
    m_pConfig->set(ck, QString(""));
    saveAndReloadConfig();

    ControlObject co(ck, true, false, true, 3.0);
    EXPECT_DOUBLE_EQ(3.0, co.get());
}

TEST_F(ControlObjectTest, Persistence_ValidValue) {
    ConfigKey ck("[Test]", "persist");
    m_pConfig->set(ck, QString("5"));
    saveAndReloadConfig();

    ControlObject co(ck, true, false, true, 3.0);
    EXPECT_DOUBLE_EQ(5.0, co.get());
}

} // namespace
