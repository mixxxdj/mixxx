#include "test/baseeffecttest.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

void BaseEffectTest::registerTestEffect(EffectManifestPointer pManifest, bool willAddToEngine) {
    MockEffectProcessor* pProcessor = new MockEffectProcessor();
    MockEffectInstantiator* pInstantiator = new MockEffectInstantiator();

    if (willAddToEngine) {
        EXPECT_CALL(*pInstantiator, instantiate(_, _))
                .Times(1)
                .WillOnce(Return(pProcessor));
    }

    m_pTestBackend->registerEffect(pManifest->id(),
            pManifest,
            EffectInstantiatorPointer(pInstantiator));
}
