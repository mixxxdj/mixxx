#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "effects/effectchain.h"
#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"

#include "test/testeffectbackend.h"

using ::testing::Return;
using ::testing::_;

class EffectsManagerTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pEffectsManager.reset(new EffectsManager(NULL, config()));
        m_pTestBackend = new TestEffectBackend();
        m_pEffectsManager->addEffectsBackend(m_pTestBackend);
    }

    // Deleted by EffectsManager. Do not delete.
    TestEffectBackend* m_pTestBackend;
    QScopedPointer<EffectsManager> m_pEffectsManager;
};

TEST_F(EffectsManagerTest, CanInstantiateEffectsFromBackend) {
    EffectManifest manifest;
    manifest.setId("org.mixxx.test.effect");
    manifest.setName("Test Effect");

    MockEffectProcessor* pProcessor = new MockEffectProcessor();
    MockEffectInstantiator* pInstantiator = new MockEffectInstantiator();

    EXPECT_CALL(*pInstantiator, instantiate(_, _))
            .Times(1)
            .WillOnce(Return(pProcessor));

    m_pTestBackend->registerEffect(manifest.id(), manifest,
                                   EffectInstantiatorPointer(pInstantiator));

    // Check we can get the same manifest that we registered back.
    EffectManifest effect_to_load = m_pEffectsManager->getEffectManifest(manifest.id());
    EXPECT_QSTRING_EQ(effect_to_load.name(), manifest.name());

    // Check we can instantiate the effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(manifest.id());
    EXPECT_FALSE(pEffect.isNull());
}
