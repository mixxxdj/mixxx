#include "effects/backends/builtin/echoeffect.h"

#include <gtest/gtest.h>

#include <QMap>
#include <QString>

#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/engine.h"
#include "test/mixxxtest.h"
#include "util/samplebuffer.h"

namespace {

class EchoEffectTest : public MixxxTest {
  protected:
    EchoEffectTest()
            : m_engineParameters(mixxx::audio::SampleRate(44100), 128),
              m_state(m_engineParameters) {
        EffectManifestPointer pManifest = EchoEffect::getManifest();
        QMap<QString, EngineEffectParameterPointer> parameterMap;
        for (const auto& pParamManifest : pManifest->parameters()) {
            parameterMap.insert(
                    pParamManifest->id(),
                    EngineEffectParameterPointer(
                            new EngineEffectParameter(pParamManifest)));
        }
        m_echoEffect.loadEngineEffectParameters(parameterMap);
        m_pDelayParameter = parameterMap.value(QStringLiteral("delay_time"));
        m_pQuantizeParameter = parameterMap.value(QStringLiteral("quantize"));
        m_pTripletParameter = parameterMap.value(QStringLiteral("triplet"));
    }

    mixxx::EngineParameters m_engineParameters;
    EchoGroupState m_state;
    EchoEffect m_echoEffect;
    EngineEffectParameterPointer m_pDelayParameter;
    EngineEffectParameterPointer m_pQuantizeParameter;
    EngineEffectParameterPointer m_pTripletParameter;
};

// Regression test for issue #13324: Setting very low BPM (e.g. 1 BPM) or long
// beat lengths must clamp delay_seconds to EchoGroupState::kMaxDelaySeconds
// without triggering DEBUG_ASSERT(delay_samples <= pGroupState->delay_buf.size()).
TEST_F(EchoEffectTest, ExtremeLowBpmDelayClamping) {
    mixxx::SampleBuffer inBuf(m_engineParameters.samplesPerBuffer());
    mixxx::SampleBuffer outBuf(m_engineParameters.samplesPerBuffer());
    inBuf.clear();
    outBuf.clear();

    GroupFeatureState groupFeatures;
    GroupFeatureBeatLength beatLength;
    beatLength.seconds = 60.0; // 1 BPM
    beatLength.scratch_rate = 1.0;
    groupFeatures.beat_length = beatLength;

    // Set delay time to 2.0 beats (which would evaluate to 120s at 1 BPM)
    m_pDelayParameter->setValue(2.0);

    EXPECT_NO_FATAL_FAILURE(m_echoEffect.processChannel(
            &m_state,
            inBuf.data(),
            outBuf.data(),
            m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures));
}

TEST_F(EchoEffectTest, StandardBpmPreserved) {
    mixxx::SampleBuffer inBuf(m_engineParameters.samplesPerBuffer());
    mixxx::SampleBuffer outBuf(m_engineParameters.samplesPerBuffer());
    inBuf.clear();
    outBuf.clear();

    GroupFeatureState groupFeatures;
    GroupFeatureBeatLength beatLength;
    beatLength.seconds = 0.5; // 120 BPM
    beatLength.scratch_rate = 1.0;
    groupFeatures.beat_length = beatLength;

    m_pDelayParameter->setValue(0.5); // 0.5 beats = 0.25s delay

    EXPECT_NO_FATAL_FAILURE(m_echoEffect.processChannel(
            &m_state,
            inBuf.data(),
            outBuf.data(),
            m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures));
}

TEST_F(EchoEffectTest, TimeModeWithoutBeatLength) {
    mixxx::SampleBuffer inBuf(m_engineParameters.samplesPerBuffer());
    mixxx::SampleBuffer outBuf(m_engineParameters.samplesPerBuffer());
    inBuf.clear();
    outBuf.clear();

    // No beat_length present; uses time parameter in seconds
    GroupFeatureState groupFeatures;

    m_pDelayParameter->setValue(2.0); // 2 seconds delay

    EXPECT_NO_FATAL_FAILURE(m_echoEffect.processChannel(
            &m_state,
            inBuf.data(),
            outBuf.data(),
            m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures));
}

} // namespace
