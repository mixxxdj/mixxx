// A series of unit tests for the widget stack, which has lots of different
// ways to control its state.
#include "widget/wwidgetstack.h"

#include <gtest/gtest.h>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "control/controlproxy.h"

class WWidgetStackTest : public MixxxTest {
  public:
    WWidgetStackTest()
          : m_pGroup("[Channel1]") {
    }

  protected:
    void SetUp() override {
        // Create a widget stack with three pages, and *before* the pages
        // are added, set the current page control to the second page.
        m_pPrevControl.reset(
                new ControlPushButton(ConfigKey(m_pGroup, "prev")));
        m_pNextControl.reset(
                new ControlPushButton(ConfigKey(m_pGroup, "next")));
        m_pCurPageControl.reset(
                new ControlObject(ConfigKey(m_pGroup, "page")));
        m_pStack.reset(new WWidgetStack(NULL, m_pNextControl.data(),
                                        m_pPrevControl.data(),
                                        m_pCurPageControl.data()));

        // 0-based index
        m_pCurPageControl->set(1);

        m_pPage0Widget = new QWidget();
        m_pPage0Control.reset(new ControlObject(ConfigKey(m_pGroup, "page0")));
        m_pPage1Widget = new QWidget();
        m_pPage1Control.reset(new ControlObject(ConfigKey(m_pGroup, "page1")));
        m_pPage2Widget = new QWidget();
        m_pPage2Control.reset(new ControlObject(ConfigKey(m_pGroup, "page2")));

        m_pStack->addWidgetWithControl(m_pPage0Widget,
                                       m_pPage0Control.data(),
                                       2);
        m_pStack->addWidgetWithControl(m_pPage1Widget,
                                       m_pPage1Control.data(),
                                       -1);
        m_pStack->addWidgetWithControl(m_pPage2Widget,
                                       m_pPage2Control.data(),
                                       -1);
        m_pStack->Init();
        m_pStack->show();
    }

    void ExpectPageSelected(int index) {
        EXPECT_EQ(index, m_pCurPageControl->get());
        EXPECT_EQ(index == 0 ? 1 : 0, m_pPage0Control->get());
        EXPECT_EQ(index == 1 ? 1 : 0, m_pPage1Control->get());
        EXPECT_EQ(index == 2 ? 1 : 0, m_pPage2Control->get());
        switch (index) {
        case 0:
            EXPECT_FALSE(m_pPage0Widget->isHidden());
            EXPECT_TRUE(m_pPage1Widget->isHidden());
            EXPECT_TRUE(m_pPage2Widget->isHidden());
            break;
        case 1:
            EXPECT_TRUE(m_pPage0Widget->isHidden());
            EXPECT_FALSE(m_pPage1Widget->isHidden());
            EXPECT_TRUE(m_pPage2Widget->isHidden());
            break;
        case 2:
            EXPECT_TRUE(m_pPage0Widget->isHidden());
            EXPECT_TRUE(m_pPage1Widget->isHidden());
            EXPECT_FALSE(m_pPage2Widget->isHidden());
            break;
        }
    }

    QScopedPointer<WWidgetStack> m_pStack;
    QScopedPointer<ControlObject> m_pPrevControl;
    QScopedPointer<ControlObject> m_pNextControl;
    QScopedPointer<ControlObject> m_pCurPageControl;
    // WWidgetStack takes ownership of the actual page widgets.
    QWidget* m_pPage0Widget;
    QWidget* m_pPage1Widget;
    QWidget* m_pPage2Widget;
    QScopedPointer<ControlObject> m_pPage0Control;
    QScopedPointer<ControlObject> m_pPage1Control;
    QScopedPointer<ControlObject> m_pPage2Control;
    const char* m_pGroup;
};

TEST_F(WWidgetStackTest, MaintainPageSelected) {
    // As a widget stack is created, if the current page control object
    // is already set, that setting should remain the same and the final stack
    // should be consistent.  Since we set the control in SetUp, that part
    // is already done.  We just need to test the current state.
    ExpectPageSelected(1);
}

TEST_F(WWidgetStackTest, MaintainPageControlValue) {
    // The current page control overrides whatever values the individual
    // page triggers may already have.  This is what caused the bug with the
    // LateNight skin -- The page trigger was set to off, but the current page
    // control was defaulted to 0, so that overrode the page trigger and
    // showed the first page.

    // This test is set up to reproduce the original LateNight skin case.
    m_pCurPageControl.reset(
            new ControlObject(ConfigKey(m_pGroup,
                                        "MaintainPageControlValue-page")));
    QScopedPointer<WWidgetStack> stack(
            new WWidgetStack(NULL, m_pNextControl.data(),
                             m_pPrevControl.data(),
                             m_pCurPageControl.data()));

    QWidget page0;
    QWidget page1;

    // We don't set the current page control.  All we know is that page0
    // is supposed to be off.
    m_pPage0Control->set(0);

    stack->addWidgetWithControl(&page0, m_pPage0Control.data(), -1);
    stack->addWidgetWithControl(&page1, NULL, -1);
    stack->Init();

    // The off state above is overridden by the default value of curpagecontrol,
    // which is set to 0 on creation.
    ExpectPageSelected(0);
}

TEST_F(WWidgetStackTest, ChangePageSelection) {
    // Changing the page via the current page control should work.
    m_pCurPageControl->set(2);
    ExpectPageSelected(2);
    m_pCurPageControl->set(0);
    ExpectPageSelected(0);
}

TEST_F(WWidgetStackTest, ChangeChildControls) {
    // Changing the page via the individual page controls should work.
    m_pPage0Control->set(1);
    ExpectPageSelected(0);
    m_pPage2Control->set(1);
    ExpectPageSelected(2);
}

TEST_F(WWidgetStackTest, OnHideBehavior) {
    // When hiding a page, if the on_hide_select value is -1, the next
    // page is selected.  Otherwise, the page given by on_hide_select is
    // selected.  The on-hide values are set in SetUp.

    // When the second page is hidden, show the third.
    m_pPage1Control->set(0);
    ExpectPageSelected(2);

    // When the last page is hidden, show the first.
    m_pPage2Control->set(0);
    ExpectPageSelected(0);

    // The first page has an override, so when it is hidden the third
    // is shown instead of the second.
    m_pPage0Control->set(0);
    ExpectPageSelected(2);
}

TEST_F(WWidgetStackTest, NextPrevControls) {
    // The next / prev controls should work, and wrap around.
    m_pNextControl->set(1);
    m_pNextControl->set(0);
    ExpectPageSelected(2);

    m_pNextControl->set(1);
    m_pNextControl->set(0);
    ExpectPageSelected(0);

    m_pNextControl->set(1);
    m_pNextControl->set(0);
    ExpectPageSelected(1);

    m_pPrevControl->set(1);
    m_pPrevControl->set(0);
    ExpectPageSelected(0);

    m_pPrevControl->set(1);
    m_pPrevControl->set(0);
    ExpectPageSelected(2);
}

TEST_F(WWidgetStackTest, HiddenStackNoChanges) {
    // When the widgetstack is hidden, it does not respond to outside changes.
    // This helps LateNight use the same triggers in multiple views.
    ExpectPageSelected(1);
    m_pStack->hide();
    m_pPage2Control->set(1);

    EXPECT_EQ(1, m_pCurPageControl->get());
    EXPECT_EQ(0, m_pPage0Control->get());
    EXPECT_EQ(1, m_pPage1Control->get());
    EXPECT_EQ(1, m_pPage2Control->get());
    EXPECT_TRUE(m_pPage0Widget->isHidden());
    EXPECT_FALSE(m_pPage1Widget->isHidden());
    EXPECT_TRUE(m_pPage2Widget->isHidden());

    // As soon as we show, the states return to being consistent, based on
    // whatever the current page control is set to.
    m_pCurPageControl->set(2);
    m_pStack->show();
    ExpectPageSelected(2);
}
