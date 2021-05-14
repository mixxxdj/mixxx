#pragma once

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
    TestEffectBackend() : EffectsBackend(NULL, EffectBackendType::Unknown) {
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
    MockEffectProcessor() {}

    MOCK_METHOD3(initialize, void(const QSet<ChannelHandleAndGroup>& activeInputChannels,
                                  EffectsManager* pEffectsManager,
                                  const mixxx::EngineParameters& bufferParameters));
    MOCK_METHOD1(createState, EffectState*(const mixxx::EngineParameters& bufferParameters));
    MOCK_METHOD2(loadStatesForInputChannel, bool(const ChannelHandle* inputChannel,
          const EffectStatesMap* pStatesMap));
    MOCK_METHOD1(deleteStatesForInputChannel, void(const ChannelHandle* inputChannel));
    MOCK_METHOD7(process, void(const ChannelHandle& inputHandle,
                               const ChannelHandle& outputHandle,
                               const CSAMPLE* pInput,
                               CSAMPLE* pOutput,
                               const mixxx::EngineParameters& bufferParameters,
                               const EffectEnableState enableState,
                               const GroupFeatureState& groupFeatures));

};

class MockEffectInstantiator : public EffectInstantiator {
  public:
    MockEffectInstantiator() {}
    MOCK_METHOD2(instantiate, EffectProcessor*(EngineEffect* pEngineEffect,
                                               EffectManifestPointer pManifest));
};


class BaseEffectTest : public MixxxTest {
  protected:
    BaseEffectTest()
            : m_pChannelHandleFactory(std::make_shared<ChannelHandleFactory>()),
              m_pTestBackend(nullptr),
              m_pEffectsManager(new EffectsManager(nullptr, config(), m_pChannelHandleFactory)) {
    }

    void registerTestBackend() {
        m_pTestBackend = new TestEffectBackend();
        m_pEffectsManager->addEffectsBackend(m_pTestBackend);
    }

    void registerTestEffect(EffectManifestPointer pManifest, bool willAddToEngine);

    ChannelHandleFactoryPointer m_pChannelHandleFactory;

    // Deleted by EffectsManager. Do not delete.
    TestEffectBackend* m_pTestBackend;
    QScopedPointer<EffectsManager> m_pEffectsManager;
};
