#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
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

} // namespace
