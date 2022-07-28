#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QScopedPointer>

#include "effects/effectinstantiator.h"
#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/effectsbackend.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "test/mixxxtest.h"

class TestEffectBackend : public EffectsBackend {
  public:
    TestEffectBackend()
            : EffectsBackend(NULL, EffectBackendType::Unknown) {
    }

    // Expose as public
    void registerEffect(const QString& id,
            EffectManifestPointer pManifest,
            EffectInstantiatorPointer pInstantiator) {
        EffectsBackend::registerEffect(id, pManifest, pInstantiator);
    }
};

class MockEffectProcessor : public EffectProcessor {
  public:
    MockEffectProcessor() {
    }

    MOCK_METHOD3(initialize,
            void(const QSet<GroupHandle>& activeInputChannels,
                    const QSet<GroupHandle>& registeredOutputChannels,
                    const mixxx::EngineParameters& engineParameters));
    MOCK_METHOD1(createState, EffectState*(const mixxx::EngineParameters& engineParameters));
    MOCK_METHOD2(loadStatesForInputChannel,
            bool(GroupHandle inputChannel,
                    const EffectStatesMap* pStatesMap));
    MOCK_METHOD1(deleteStatesForInputChannel, void(GroupHandle inputChannel));
    MOCK_METHOD7(process,
            void(GroupHandle inputHandle,
                    GroupHandle outputHandle,
                    const CSAMPLE* pInput,
                    CSAMPLE* pOutput,
                    const mixxx::EngineParameters& engineParameters,
                    const EffectEnableState enableState,
                    const GroupFeatureState& groupFeatures));
};

class MockEffectInstantiator : public EffectInstantiator {
  public:
    MockEffectInstantiator() {
    }
    MOCK_METHOD2(instantiate,
            EffectProcessor*(EngineEffect* pEngineEffect,
                    EffectManifestPointer pManifest));
};

class BaseEffectTest : public MixxxTest {
  protected:
    BaseEffectTest()
            : m_pTestBackend(nullptr),
              m_pEffectsManager(new EffectsManager(nullptr, config())) {
    }

    void registerTestBackend() {
        m_pTestBackend = new TestEffectBackend();
        m_pEffectsManager->addEffectsBackend(m_pTestBackend);
    }

    void registerTestEffect(EffectManifestPointer pManifest, bool willAddToEngine);

    // Deleted by EffectsManager. Do not delete.
    TestEffectBackend* m_pTestBackend;
    QScopedPointer<EffectsManager> m_pEffectsManager;
};
