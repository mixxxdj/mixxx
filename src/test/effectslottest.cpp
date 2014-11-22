#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "effects/effectchain.h"
#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"

#include "test/baseeffecttest.h"

using ::testing::Return;
using ::testing::_;

class EffectSlotTest : public BaseEffectTest {
  protected:
    virtual void SetUp() {
        m_pEffectsManager->registerGroup("[Master]");
        m_pEffectsManager->registerGroup("[Headphone]");
        registerTestBackend();
    }
};

TEST_F(EffectSlotTest, ControlsReflectSlotState) {
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                              "org.mixxx.test.chain1"));
    int iRackNumber = 0;
    int iChainNumber = 0;
    int iEffectNumber = 0;

    StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
    EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlot();
    // StandardEffectRack::addEffectChainSlot automatically adds 4 effect
    // slots. In the future we will probably remove this so this will just start
    // segfaulting.
    EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(0);

    QString group = StandardEffectRack::formatEffectSlotGroupString(
        iRackNumber, iChainNumber, iEffectNumber);

    EffectManifest manifest;
    manifest.setId("org.mixxx.test.effect");
    manifest.setName("Test Effect");
    manifest.addParameter();
    registerTestEffect(manifest, false);

    // Check the controls reflect the state of their loaded effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(manifest.id());

    // Enabled defaults to true in both effects and the slot.
    pEffect->setEnabled(false);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(group, "enabled")));
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(group, "num_parameters")));

    pEffectSlot->loadEffect(pEffect);
    EXPECT_LE(0, ControlObject::get(ConfigKey(group, "enabled")));
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "num_parameters")));
    EXPECT_TRUE(pEffect->enabled());

    // loaded is read-only.
    ControlObject::set(ConfigKey(group, "loaded"), 0.0);
    EXPECT_LE(0, ControlObject::get(ConfigKey(group, "loaded")));

    // num_parameters is read-only.
    ControlObject::set(ConfigKey(group, "num_parameters"), 2.0);
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "num_parameters")));
}
