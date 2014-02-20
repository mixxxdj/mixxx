#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "effects/effectchain.h"
#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"

using ::testing::Return;
using ::testing::_;

class EffectChainSlotTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pEffectsManager.reset(new EffectsManager(NULL, config()));
        m_pEffectsManager->registerGroup("[Master]");
        m_pEffectsManager->registerGroup("[Headphone]");
    }

    QScopedPointer<EffectsManager> m_pEffectsManager;
};

TEST_F(EffectChainSlotTest, ChainSlotMirrorsLoadedChain) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));
    int iRackNumber = 0;
    int iChainNumber = 0;

    EffectRackPointer pRack = m_pEffectsManager->addEffectRack();
    EffectChainSlotPointer pSlot = pRack->addEffectChainSlot();

    QString group = EffectChainSlot::formatGroupString(iRackNumber,
                                                       iChainNumber);
    pSlot->loadEffectChain(pChain);

    pChain->setEnabled(true);
    EXPECT_LT(0.0, ControlObject::get(ConfigKey(group, "enabled")));

    pChain->setEnabled(false);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(group, "enabled")));

    // Enabled is read-only. Sets to it should not do anything.
    ControlObject::set(ConfigKey(group, "enabled"), 1);
    EXPECT_FALSE(pChain->enabled());

    // numEffects is read-only. Sets to it should not do anything.
    ControlObject::set(ConfigKey(group, "num_effects"), 1);
    EXPECT_EQ(0, pChain->numEffects());

    ControlObject::set(ConfigKey(group, "parameter"), 0.5);
    EXPECT_DOUBLE_EQ(0.5, pChain->parameter());

    pChain->setParameter(1.0);
    EXPECT_DOUBLE_EQ(pChain->parameter(),
                     ControlObject::get(ConfigKey(group, "parameter")));

    ControlObject::set(ConfigKey(group, "parameter"), 0.5);
    EXPECT_DOUBLE_EQ(0.5, pChain->parameter());

    pChain->setMix(1.0);
    EXPECT_DOUBLE_EQ(pChain->mix(),
                     ControlObject::get(ConfigKey(group, "mix")));

    ControlObject::set(ConfigKey(group, "mix"), 0.5);
    EXPECT_DOUBLE_EQ(0.5, pChain->mix());

    pChain->setInsertionType(EffectChain::SEND);
    EXPECT_DOUBLE_EQ(pChain->insertionType(),
                     ControlObject::get(ConfigKey(group, "insertion_type")));

    ControlObject::set(ConfigKey(group, "insertion_type"), EffectChain::INSERT);
    EXPECT_DOUBLE_EQ(EffectChain::INSERT, pChain->insertionType());

    EXPECT_FALSE(pChain->enabledForGroup("[Master]"));
    pChain->enableForGroup("[Master]");
    EXPECT_LT(0.0, ControlObject::get(ConfigKey(group, "channel_[Master]")));

    ControlObject::set(ConfigKey(group, "channel_[Master]"), 0);
    EXPECT_FALSE(pChain->enabledForGroup("[Master]"));
}

TEST_F(EffectChainSlotTest, ChainSlotMirrorsLoadedChain_Clear) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));

    int iRackNumber = 0;
    int iChainNumber = 0;

    EffectRackPointer pRack = m_pEffectsManager->addEffectRack();
    EffectChainSlotPointer pSlot = pRack->addEffectChainSlot();

    QString group = EffectChainSlot::formatGroupString(iRackNumber,
                                                       iChainNumber);
    EXPECT_FALSE(pChain->enabled());
    pSlot->loadEffectChain(pChain);
    EXPECT_TRUE(pChain->enabled());
    ControlObject::set(ConfigKey(group, "clear"), 1.0);
    EXPECT_FALSE(pChain->enabled());
}
