#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "controlpushbutton.h"
#include "widget/wpushbutton.h"
#include "widget/controlwidgetconnection.h"

class WPushButtonTest : public MixxxTest {
  public:
    WPushButtonTest()
          : m_pGroup("[Channel1]") {
    }

  protected:
    virtual void SetUp() {
        m_pButton.reset(new WPushButton());
        m_pButton->setStates(2);
    }

    QScopedPointer<WPushButton> m_pButton;
    QTestEventList m_Events;
    const char* m_pGroup;
};

TEST_F(WPushButtonTest, QuickPressNoLatchTest) {
    QScopedPointer<ControlPushButton> pPushControl(
        new ControlPushButton(ConfigKey("[Test]", "push")));
    pPushControl->setButtonMode(ControlPushButton::LONGPRESSLATCHING);

    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LONGPRESSLATCHING,
                                    ControlPushButton::PUSH));
    m_pButton->setStates(2);
    m_pButton->addLeftConnection(
        new ControlParameterWidgetConnection(
            m_pButton.data(),
            new ControlObjectSlave(pPushControl->getKey()), NULL,
            ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET,
            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE));

    // This test can be flaky if the event simulator takes too long to deliver
    // the event.
    m_Events.addMousePress(Qt::LeftButton);
    m_Events.addMouseRelease(Qt::LeftButton, 0, QPoint(), 1);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(0.0, m_pButton->getControlParameterLeft());
}

TEST_F(WPushButtonTest, LongPressLatchTest) {
    QScopedPointer<ControlPushButton> pPushControl(
        new ControlPushButton(ConfigKey("[Test]", "push")));
    pPushControl->setButtonMode(ControlPushButton::LONGPRESSLATCHING);

    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LONGPRESSLATCHING,
                                    ControlPushButton::PUSH));
    m_pButton->setStates(2);
    m_pButton->addLeftConnection(
        new ControlParameterWidgetConnection(
            m_pButton.data(),
            new ControlObjectSlave(pPushControl->getKey()), NULL,
            ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET,
            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE));

    m_Events.addMousePress(Qt::LeftButton);
    m_Events.addMouseRelease(Qt::LeftButton, 0, QPoint(), 1000);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(1.0, m_pButton->getControlParameterLeft());
}
