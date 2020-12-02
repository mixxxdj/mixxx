#include "test/baseeffecttest.h"

#include <gmock/gmock-actions.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-actions.h>
#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>

#include <QSharedPointer>

#include "effects/effectmanifest.h"

using ::testing::Return;
using ::testing::Invoke;
using ::testing::_;

void BaseEffectTest::registerTestEffect(EffectManifestPointer pManifest, bool willAddToEngine) {
    MockEffectProcessor* pProcessor = new MockEffectProcessor();
    MockEffectInstantiator* pInstantiator = new MockEffectInstantiator();

    if (willAddToEngine) {
        EXPECT_CALL(*pInstantiator, instantiate(_, _))
                .Times(1)
                .WillOnce(Return(pProcessor));
    }

    m_pTestBackend->registerEffect(pManifest->id(), pManifest,
                                   EffectInstantiatorPointer(pInstantiator));
}
