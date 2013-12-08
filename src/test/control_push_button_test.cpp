#include <gtest/gtest.h>

#include <QApplication>
#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "widget/wpushbutton.h"

class ControlPushButtonTest : public MixxxTest {
  public:
    ControlPushButtonTest()
          : m_pGroup("[Channel1]"),
            m_argc(1),
            m_argv("testApp") {
        m_pApplication.reset(new QApplication(m_argc, m_argv));
    }

  protected:
    virtual void SetUp() {
        m_pButton.reset(new WPushButton());
        m_pButton->setStates(2);
    }

    int m_argc;
    const char* m_argv;
    QScopedPointer<QApplication> m_pApplication;
    QScopedPointer<WPushButton> m_pButton;
    QTestEventList m_Events;
    const char* m_pGroup;
};

TEST_F(ControlPushButtonTest, QuickPressNoLatchTest) {
    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LATCHING,
                                    ControlPushButton::PUSH));

    m_Events.addMousePress(Qt::LeftButton, 0, QPoint(), 100);
    m_Events.addMouseRelease(Qt::LeftButton);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(0.0, m_pButton->getValue());
}

TEST_F(ControlPushButtonTest, LongPressLatchTest) {
    m_pButton.reset(new WPushButton(NULL, ControlPushButton::LATCHING,
                                    ControlPushButton::PUSH));

    m_Events.addMousePress(Qt::LeftButton, 0, QPoint(), 1000);
    m_Events.addMouseRelease(Qt::LeftButton);

    m_Events.simulate(m_pButton.data());

    ASSERT_EQ(1.0, m_pButton->getValue());
}


