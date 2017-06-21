#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "effects/effectchain.h"
#include "effects/effectchainslot.h"
#include "effects/effectrack.h"
#include "effects/effectsmanager.h"
#include "test/baseeffecttest.h"

using ::testing::Return;
using ::testing::_;

class EffectChainSlotTest : public BaseEffectTest {
  protected:
    EffectChainSlotTest()
            : m_master(m_factory.getOrCreateHandle("[Master]"), "[Master]"),
              m_headphone(m_factory.getOrCreateHandle("[Headphone]"), "[Headphone]") {
        m_pEffectsManager->registerChannel(m_master);
        m_pEffectsManager->registerChannel(m_headphone);
    }

    ChannelHandleFactory m_factory;
    ChannelHandleAndGroup m_master;
    ChannelHandleAndGroup m_headphone;
};

TEST_F(EffectChainSlotTest, ChainSlotMirrorsLoadedChain) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));
    int iRackNumber = 0;
    int iChainNumber = 0;

    StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
    EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlot();

    QString group = StandardEffectRack::formatEffectChainSlotGroupString(
        iRackNumber, iChainNumber);
    pChainSlot->loadEffectChain(pChain);

    pChain->setEnabled(true);
    EXPECT_LT(0.0, ControlObject::get(ConfigKey(group, "enabled")));

    pChain->setEnabled(false);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(group, "enabled")));

    ControlObject::set(ConfigKey(group, "enabled"), 1.0);
    EXPECT_TRUE(pChain->enabled());

    // Loaded is read-only. Sets to it should not do anything.
    ControlObject::set(ConfigKey(group, "loaded"), 0);
    EXPECT_TRUE(ControlObject::get(ConfigKey(group, "loaded")) > 0.0);

    // numEffects is read-only. Sets to it should not do anything.
    ControlObject::set(ConfigKey(group, "num_effects"), 1);
    EXPECT_EQ(0U, pChain->numEffects());

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

    EXPECT_FALSE(pChain->enabledForChannel(m_master));
    pChain->enableForChannel(m_master);
    EXPECT_LT(0.0, ControlObject::get(ConfigKey(group, "group_[Master]_enable")));

    ControlObject::set(ConfigKey(group, "group_[Master]_enable"), 0);
    EXPECT_FALSE(pChain->enabledForChannel(m_master));
}

TEST_F(EffectChainSlotTest, ChainSlotMirrorsLoadedChain_StartsWithChainLoaded) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));
    int iRackNumber = 0;
    int iChainNumber = 0;

    StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
    EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlot();
    pChainSlot->loadEffectChain(pChain); 
    QString group = StandardEffectRack::formatEffectChainSlotGroupString(
        iRackNumber, iChainNumber);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(group, "loaded")));
}

TEST_F(EffectChainSlotTest, ChainSlotMirrorsLoadedChain_Clear) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));

    int iRackNumber = 0;
    int iChainNumber = 0;

    StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
    EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlot();

    QString group = StandardEffectRack::formatEffectChainSlotGroupString(
        iRackNumber, iChainNumber);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(group, "loaded")));
    pChainSlot->loadEffectChain(pChain);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(group, "loaded")));
    ControlObject::set(ConfigKey(group, "clear"), 1.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(group, "loaded")));
}
