#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <memory>

#include "control/controlobject.h"
#include "qml/qmlcontrolproxy.h"
#include "test/mixxxtest.h"

using namespace mixxx::qml;
using ::testing::ElementsAre;

namespace {

class QmlControlProxyTest : public MixxxTest {
  protected:
    void SetUp() override {
        // Use a unique key for this test
        m_key = ConfigKey("[TestQmlProxy]", "trigger_test");
        co = std::make_unique<ControlObject>(m_key);
        m_proxy = std::make_unique<QmlControlProxy>();
        m_proxy->setGroup(m_key.group);
        m_proxy->setKey(m_key.item);
        m_proxy->componentComplete();
    }

    void processEvents() {
        // Ensure all queued events are processed
        application()->processEvents();
        application()->processEvents();
    }

    ConfigKey m_key;
    std::unique_ptr<ControlObject> co;
    std::unique_ptr<QmlControlProxy> m_proxy;
};

TEST_F(QmlControlProxyTest, TriggerEmitsValueChanged1Then0) {
    QSignalSpy valueSpy(m_proxy.get(), &mixxx::qml::QmlControlProxy::valueChanged);

    // Initial value should be 0
    EXPECT_DOUBLE_EQ(m_proxy->getValue(), 0.0);

    m_proxy->trigger();
    QmlControlProxyTest::processEvents();

    // The first emission should be 1, the second should be 0
    ASSERT_GE(valueSpy.size(), 2);
    EXPECT_DOUBLE_EQ(valueSpy.at(0).at(0).toDouble(), 1.0);
    EXPECT_DOUBLE_EQ(valueSpy.at(1).at(0).toDouble(), 0.0);

    // The final value should be 0
    EXPECT_DOUBLE_EQ(m_proxy->getValue(), 0.0);
}

TEST_F(QmlControlProxyTest, ToggleSwitchesBetweenZeroAndOne) {
    m_proxy->toggle();
    processEvents();
    EXPECT_DOUBLE_EQ(m_proxy->getValue(), 1.0);

    m_proxy->toggle();
    processEvents();
    EXPECT_DOUBLE_EQ(m_proxy->getValue(), 0.0);
}

TEST_F(QmlControlProxyTest, UpdateIntervalThrottlesControlChanges) {
    m_proxy->setUpdateInterval(50);
    QSignalSpy valueSpy(m_proxy.get(), &mixxx::qml::QmlControlProxy::valueChanged);

    // Leading edge: the first change is delivered right away
    co->set(1.0);
    processEvents();
    ASSERT_EQ(valueSpy.size(), 1);
    EXPECT_DOUBLE_EQ(valueSpy.at(0).at(0).toDouble(), 1.0);

    // Changes inside the quiet window are coalesced
    co->set(2.0);
    co->set(3.0);
    processEvents();
    EXPECT_EQ(valueSpy.size(), 1);

    // ... and the latest value is delivered when the window ends
    ASSERT_TRUE(valueSpy.wait(1000));
    ASSERT_EQ(valueSpy.size(), 2);
    EXPECT_DOUBLE_EQ(valueSpy.at(1).at(0).toDouble(), 3.0);
    EXPECT_DOUBLE_EQ(m_proxy->getValue(), 3.0);
}

TEST_F(QmlControlProxyTest, UpdateIntervalZeroDeliversEveryChange) {
    QSignalSpy valueSpy(m_proxy.get(), &mixxx::qml::QmlControlProxy::valueChanged);
    co->set(1.0);
    co->set(2.0);
    co->set(3.0);
    processEvents();
    ASSERT_EQ(valueSpy.size(), 3);
    EXPECT_DOUBLE_EQ(valueSpy.at(2).at(0).toDouble(), 3.0);
}

TEST_F(QmlControlProxyTest, ResettingUpdateIntervalDeliversPendingValue) {
    m_proxy->setUpdateInterval(10000);
    QSignalSpy valueSpy(m_proxy.get(), &mixxx::qml::QmlControlProxy::valueChanged);
    co->set(1.0);
    co->set(2.0);
    processEvents();
    ASSERT_EQ(valueSpy.size(), 1);

    m_proxy->setUpdateInterval(0);
    ASSERT_EQ(valueSpy.size(), 2);
    EXPECT_DOUBLE_EQ(valueSpy.at(1).at(0).toDouble(), 2.0);
}

} // namespace
