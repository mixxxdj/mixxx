#include <gtest/gtest.h>

#include <QtDebug>

#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "test/mixxxtest.h"

namespace {

class ControlPotmeterTest : public MixxxTest {
  protected:
    void SetUp() override {
        ck = ConfigKey("[SomeGroup]", "some_controlpotmeter");
        co = std::make_unique<ControlPotmeter>(ck, -1.0, 1.0);
    }

    ConfigKey ck;
    std::unique_ptr<ControlPotmeter> co;
};

TEST_F(ControlPotmeterTest, Up) {
    auto coUp = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_up")));
    co->set(0.0);
    ASSERT_DOUBLE_EQ(0.0, co->get());

    coUp.set(1.0);
    coUp.set(0.0);
    EXPECT_GT(co->get(), 0.0);

    double previousValue = co->get();
    coUp.set(1.0);
    coUp.set(0.0);
    EXPECT_GT(co->get(), previousValue);
}

TEST_F(ControlPotmeterTest, UpSmall) {
    auto coUpSmall = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_up_small")));
    co->set(0.0);
    ASSERT_DOUBLE_EQ(0.0, co->get());

    coUpSmall.set(1.0);
    coUpSmall.set(0.0);
    EXPECT_GT(co->get(), 0.0);

    double previousValue = co->get();
    coUpSmall.set(1.0);
    coUpSmall.set(0.0);
    EXPECT_GT(co->get(), previousValue);
}

TEST_F(ControlPotmeterTest, Down) {
    auto coDown = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_down")));
    co->set(1.0);
    ASSERT_DOUBLE_EQ(1.0, co->get());

    coDown.set(1.0);
    coDown.set(0.0);
    EXPECT_LT(co->get(), 1.0);

    double previousValue = co->get();
    coDown.set(1.0);
    coDown.set(0.0);
    EXPECT_LT(co->get(), previousValue);
}

TEST_F(ControlPotmeterTest, DownSmall) {
    auto coDownSmall = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_down_small")));
    co->set(1.0);
    ASSERT_DOUBLE_EQ(1.0, co->get());

    coDownSmall.set(1.0);
    coDownSmall.set(0.0);
    EXPECT_LT(co->get(), 1.0);

    double previousValue = co->get();
    coDownSmall.set(1.0);
    coDownSmall.set(0.0);
    EXPECT_LT(co->get(), previousValue);
}

TEST_F(ControlPotmeterTest, SetDefault) {
    auto coSetDefault = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_set_default")));
    co->setDefaultValue(0.25);
    co->set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());

    coSetDefault.set(1.0);
    coSetDefault.set(0.0);
    EXPECT_DOUBLE_EQ(0.25, co->get());

    co->setDefaultValue(0.75);
    co->set(1.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coSetDefault.set(1.0);
    coSetDefault.set(0.0);
    EXPECT_DOUBLE_EQ(0.75, co->get());

    co->setDefaultValue(0.0);
}

TEST_F(ControlPotmeterTest, SetZero) {
    auto coSetZero = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_set_zero")));
    co->set(0.5);
    EXPECT_DOUBLE_EQ(0.5, co->get());

    coSetZero.set(1.0);
    coSetZero.set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());

    co->set(1.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coSetZero.set(1.0);
    coSetZero.set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControlPotmeterTest, SetOne) {
    auto coSetOne = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_set_one")));
    co->set(0.5);
    EXPECT_DOUBLE_EQ(0.5, co->get());

    coSetOne.set(1.0);
    coSetOne.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    co->set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());

    coSetOne.set(1.0);
    coSetOne.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControlPotmeterTest, SetMinusOne) {
    auto coSetMinusOne = ControlProxy(
            ConfigKey(ck.group, ck.item + QStringLiteral("_set_minus_one")));
    co->set(0.5);
    ASSERT_DOUBLE_EQ(0.5, co->get());

    coSetMinusOne.set(1.0);
    coSetMinusOne.set(0.0);
    EXPECT_DOUBLE_EQ(-1.0, co->get());

    co->set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());

    coSetMinusOne.set(1.0);
    coSetMinusOne.set(0.0);
    EXPECT_DOUBLE_EQ(-1.0, co->get());
}

TEST_F(ControlPotmeterTest, Toggle) {
    auto coToggle = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_toggle")));
    co->set(0.0);
    ASSERT_DOUBLE_EQ(0.0, co->get());

    coToggle.set(1.0);
    coToggle.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coToggle.set(1.0);
    coToggle.set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());

    co->set(-0.5);
    coToggle.set(1.0);
    coToggle.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coToggle.set(1.0);
    coToggle.set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControlPotmeterTest, MinusToggle) {
    auto coMinusToggle = ControlProxy(
            ConfigKey(ck.group, ck.item + QStringLiteral("_minus_toggle")));
    co->set(0.0);
    ASSERT_DOUBLE_EQ(0.0, co->get());

    coMinusToggle.set(1.0);
    coMinusToggle.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coMinusToggle.set(1.0);
    coMinusToggle.set(0.0);
    EXPECT_DOUBLE_EQ(-1.0, co->get());

    co->set(-0.5);
    coMinusToggle.set(1.0);
    coMinusToggle.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());

    coMinusToggle.set(1.0);
    coMinusToggle.set(0.0);
    EXPECT_DOUBLE_EQ(-1.0, co->get());
}

TEST_F(ControlPotmeterTest, AddAlias) {
    co->addAlias(ConfigKey("[AliasGroup]", "alias_controlpotmeter"));

    auto alias = ControlProxy(ConfigKey("[AliasGroup]", "alias_controlpotmeter"));
    EXPECT_DOUBLE_EQ(co->get(), alias.get());

    co->set(1.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());
    EXPECT_DOUBLE_EQ(1.0, alias.get());

    co->set(0.5);
    EXPECT_DOUBLE_EQ(0.5, co->get());
    EXPECT_DOUBLE_EQ(0.5, alias.get());

    alias.set(0.25);
    EXPECT_DOUBLE_EQ(0.25, co->get());
    EXPECT_DOUBLE_EQ(0.25, alias.get());

    auto aliasSetMinusOne = ControlProxy(
            ConfigKey("[AliasGroup]", "alias_controlpotmeter_set_one"));
    aliasSetMinusOne.set(1.0);
    aliasSetMinusOne.set(0.0);
    EXPECT_DOUBLE_EQ(1.0, co->get());
    EXPECT_DOUBLE_EQ(1.0, alias.get());

    auto coSetZero = ControlProxy(ConfigKey(ck.group, ck.item + QStringLiteral("_set_zero")));
    coSetZero.set(1.0);
    coSetZero.set(0.0);
    EXPECT_DOUBLE_EQ(0.0, co->get());
    EXPECT_DOUBLE_EQ(0.0, alias.get());
}

} // namespace
