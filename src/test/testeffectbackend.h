#ifndef TESTEFFECTBACKEND_H
#define TESTEFFECTBACKEND_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "effects/effectsbackend.h"
#include "effects/effectinstantiator.h"
#include "effects/effectprocessor.h"

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

    MOCK_METHOD4(process, void(const QString& group, const CSAMPLE* pInput,
                               CSAMPLE* pOutput,
                               const unsigned int numSamples));
};

class MockEffectInstantiator : public EffectInstantiator {
  public:
    MockEffectInstantiator() {}
    MOCK_METHOD2(instantiate, EffectProcessor*(EngineEffect* pEngineEffect,
                                               const EffectManifest& manifest));
};

#endif /* TESTEFFECTBACKEND_H */
