#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "widget/wpushbutton.h"

class WPushButtonTest : public MixxxTest {
  public:
    WPushButtonTest()
          : m_pGroup("[Channel1]"){
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
    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LONGPRESSLATCHING,
                                    ControlPushButton::PUSH));
    m_pButton->setStates(2);

    m_Events.addMousePress(Qt::LeftButton);
    m_Events.addMouseRelease(Qt::LeftButton, 0, QPoint(), 100);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(0.0, m_pButton->getValue());
}

TEST_F(WPushButtonTest, LongPressLatchTest) {
    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LONGPRESSLATCHING,
                                    ControlPushButton::PUSH));
    m_pButton->setStates(2);

    m_Events.addMousePress(Qt::LeftButton);
    m_Events.addMouseRelease(Qt::LeftButton, 0, QPoint(), 1000);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(1.0, m_pButton->getValue());
}
