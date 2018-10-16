#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "test/mixxxtest.h"
#include "effects/effectchain.h"
#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"

#include "test/baseeffecttest.h"

using ::testing::Return;
using ::testing::_;

class EffectsManagerTest : public BaseEffectTest {
  protected:
    void SetUp() override {
        registerTestBackend();
    }
};

TEST_F(EffectsManagerTest, CanInstantiateEffectsFromBackend) {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId("org.mixxx.test.effect");
    pManifest->setName("Test Effect");
    registerTestEffect(pManifest, false);

    // Check we can get the same manifest that we registered back.
    EffectManifestPointer effect_to_load = m_pEffectsManager->getEffectManifest(pManifest->id());
    EXPECT_QSTRING_EQ(effect_to_load->name(), pManifest->name());

    // Check we can instantiate the effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(pManifest->id());
    EXPECT_FALSE(pEffect.isNull());
}
