#ifndef BASEEFFECTTEST_H
#define BASEEFFECTTEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QScopedPointer>

#include "effects/effectchain.h"
#include "effects/effect.h"
#include "effects/effectsmanager.h"
#include "effects/effectmanifest.h"
#include "effects/effectsbackend.h"
#include "effects/effectinstantiator.h"
#include "effects/effectprocessor.h"


#include "test/mixxxtest.h"

class TestEffectBackend : public EffectsBackend {
  public:
    TestEffectBackend() : EffectsBackend(NULL, "TestBackend") {
    }

    // Expose as public
    void registerEffect(const QString& id,
                        const EffectManifest& manifest,
                        EffectInstantiatorPointer pInstantiator) {
        EffectsBackend::registerEffect(id, manifest, pInstantiator);
    }
};

class MockEffectProcessor : public EffectProcessor {
  public:
    MockEffectProcessor() {}

    MOCK_METHOD7(process, void(const QString& group, const CSAMPLE* pInput,
                               CSAMPLE* pOutput,
                               const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const EffectProcessor::EnableState enableState,
                               const GroupFeatureState& groupFeatures));

    MOCK_METHOD1(initialize, void(const QSet<QString>& registeredGroups));
};

class MockEffectInstantiator : public EffectInstantiator {
  public:
    MockEffectInstantiator() {}
    MOCK_METHOD2(instantiate, EffectProcessor*(EngineEffect* pEngineEffect,
                                               const EffectManifest& manifest));
};


class BaseEffectTest : public MixxxTest {
  protected:
    BaseEffectTest() : m_pTestBackend(NULL),
                       m_pEffectsManager(new EffectsManager(NULL, config())) {
    }

    void registerTestBackend() {
        m_pTestBackend = new TestEffectBackend();
        m_pEffectsManager->addEffectsBackend(m_pTestBackend);
    }

    void registerTestEffect(const EffectManifest& manifest, bool willAddToEngine);

    // Deleted by EffectsManager. Do not delete.
    TestEffectBackend* m_pTestBackend;
    QScopedPointer<EffectsManager> m_pEffectsManager;
};


#endif /* BASEEFFECTTEST_H */
