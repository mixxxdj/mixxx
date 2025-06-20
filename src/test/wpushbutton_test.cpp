#include "widget/wpushbutton.h"

#include <gtest/gtest.h>

#include <QTestEventList>
#include <memory>

#include "control/controlpushbutton.h"
#include "util/valuetransformer.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wbasewidget.h"

class WPushButtonTest : public ::testing::Test {
  public:
    // touchShift is needed internally to avoid a DEBUG_ASSERT
    ControlPushButton touchShift = ConfigKey(
            QStringLiteral("[Controls]"), QStringLiteral("touch_shift"));
    ControlPushButton pushControl = ConfigKey(QStringLiteral("[Test]"), QStringLiteral("push"));
    WPushButton pushButton = WPushButton(nullptr,
            mixxx::control::ButtonMode::LongPressLatching,
            mixxx::control::ButtonMode::Push);

    WPushButtonTest() {
        pushControl.setButtonMode(mixxx::control::ButtonMode::LongPressLatching);
        pushButton.setStates(2);
        pushButton.addConnection(
                std::make_unique<ControlParameterWidgetConnection>(
                        &pushButton,
                        pushControl.getKey(),
                        nullptr,
                        ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET,
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
                WBaseWidget::ConnectionSide::Left);
    }
};

TEST_F(WPushButtonTest, QuickPressNoLatchTest) {
    // This test can be flaky if the event simulator takes too long to deliver
    // the event.
    QTestEventList events;
    events.addMousePress(Qt::LeftButton);
    events.addDelay(100);
    events.addMouseRelease(Qt::LeftButton);

    events.simulate(&pushButton);

    ASSERT_EQ(0.0, pushButton.getControlParameterLeft());
}

TEST_F(WPushButtonTest, LongPressLatchTest) {
    QTestEventList events;
    events.addMousePress(Qt::LeftButton);
    events.addDelay(1000);
    events.addMouseRelease(Qt::LeftButton);

    events.simulate(&pushButton);

    ASSERT_EQ(1.0, pushButton.getControlParameterLeft());
}
