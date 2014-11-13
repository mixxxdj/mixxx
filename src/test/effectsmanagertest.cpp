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
    virtual void SetUp() {
        registerTestBackend();
    }
};

TEST_F(EffectsManagerTest, CanInstantiateEffectsFromBackend) {
    EffectManifest manifest;
    manifest.setId("org.mixxx.test.effect");
    manifest.setName("Test Effect");
    registerTestEffect(manifest, false);

    // Check we can get the same manifest that we registered back.
    EffectManifest effect_to_load = m_pEffectsManager->getEffectManifest(manifest.id());
    EXPECT_QSTRING_EQ(effect_to_load.name(), manifest.name());

    // Check we can instantiate the effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(manifest.id());
    EXPECT_FALSE(pEffect.isNull());
}
