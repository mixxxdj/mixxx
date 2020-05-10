#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectslot.h"

#include "test/baseeffecttest.h"

using ::testing::Return;
using ::testing::_;

class EffectSlotTest : public BaseEffectTest {
  protected:
    EffectSlotTest()
            : m_master(m_factory.getOrCreateHandle("[Master]"), "[Master]"),
              m_headphone(m_factory.getOrCreateHandle("[Headphone]"), "[Headphone]") {
        registerTestBackend();
    }

    ChannelHandleFactory m_factory;
    ChannelHandleAndGroup m_master;
    ChannelHandleAndGroup m_headphone;
};

TEST_F(EffectSlotTest, ControlsReflectSlotState) {
    int iChainNumber = 0;
    int iEffectNumber = 0;

    m_pEffectsManager->addStandardEffectChainSlots();
    EffectChainSlotPointer pChainSlot = m_pEffectsManager->getStandardEffectChainSlot(iChainNumber);
    // StandardEffectChainSlot::addEffectChainSlot automatically adds 4 effect
    // slots. In the future we will probably remove this so this will just start
    // segfaulting.
    EffectSlotPointer pEffectSlot = pChainSlot->getEffectSlot(0);

    QString group = StandardEffectChainSlot::formatEffectSlotGroup(
        iChainNumber, iEffectNumber);

    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId("org.mixxx.test.effect");
    pManifest->setName("Test Effect");
    pManifest->addParameter();
    registerTestEffect(pManifest, false);

    // Enabled defaults to false in effect, slot, and engine effect.
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(group, "enabled")));
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(group, "num_parameters")));

    m_pEffectsManager->loadEffect(pChainSlot, iEffectNumber,
            pManifest->id(), EffectBackendType::Unknown);
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(group, "enabled")));
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "num_parameters")));

    pEffectSlot->setEnabled(true);
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "enabled")));
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "num_parameters")));

    // loaded is read-only.
    ControlObject::set(ConfigKey(group, "loaded"), 0.0);
    EXPECT_LE(0, ControlObject::get(ConfigKey(group, "loaded")));

    // num_parameters is read-only.
    ControlObject::set(ConfigKey(group, "num_parameters"), 2.0);
    EXPECT_DOUBLE_EQ(1, ControlObject::get(ConfigKey(group, "num_parameters")));
}
