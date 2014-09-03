#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "effects/effectparameterslot.h"
//#include "controlobject.h"
//#include "controlpushbutton.h"
//#include "controlobjectthread.h"
//#include "engine/loopingcontrol.h"
//#include "test/mockedenginebackendtest.h"

class SuperLinkTest : public MixxxTest {
  public:
    SuperLinkTest() {
    }

  protected:
    virtual void SetUp() {
        m_pParameterSlot.reset(new EffectParameterSlot(0, 0, 0, 0));

        QString group = EffectParameterSlotBase::formatGroupString(0, 0, 0);
        QString itemPrefix = EffectParameterSlot::formatItemPrefix(0);

        m_pControlValue.reset(new ControlObjectThread(group, itemPrefix));

        m_pControlLinkType.reset(new ControlObjectThread(group,
                itemPrefix + QString("_link_type")));

        m_pControlLinkInverse.reset(new ControlObjectThread(group,
                itemPrefix + QString("_link_inverse")));
    }

    QScopedPointer<EffectParameterSlot> m_pParameterSlot;

    QScopedPointer<ControlObjectThread> m_pControlValue;
    QScopedPointer<ControlObjectThread> m_pControlLinkType;
    QScopedPointer<ControlObjectThread> m_pControlLinkInverse;

};

TEST_F(SuperLinkTest, LinkDefault) {
    // default is not Linked, value must be unchanged
    EXPECT_EQ(0, m_pControlValue->get());
    m_pParameterSlot->onChainParameterChanged(1.0);
    EXPECT_EQ(0, m_pControlValue->get());
    m_pParameterSlot->onChainParameterChanged(0.5);
    EXPECT_EQ(0, m_pControlValue->get());
    m_pParameterSlot->onChainParameterChanged(0.3);
    EXPECT_EQ(0, m_pControlValue->get());
}
