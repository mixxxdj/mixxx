#include <gtest/gtest.h>

#include <QtDebug>

#include "effects/backends/builtin/trackereffect.h"
#include "effects/backends/effectmanifest.h"
#include "engine/engine.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/effects/groupfeaturestate.h"
#include "test/mixxxtest.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

namespace mixxx {

namespace {

constexpr SINT kTestBufferSize = 256;
const audio::SampleRate kTestSampleRate = audio::SampleRate(44100);

// Creates EngineEffectParameters from the TrackerEffect manifest and loads
// them into the effect, so processChannel can read parameter values.
QMap<QString, EngineEffectParameterPointer> createAndLoadParameters(
        TrackerEffect* pEffect) {
    EffectManifestPointer pManifest = TrackerEffect::getManifest();
    QMap<QString, EngineEffectParameterPointer> parameters;
    for (const auto& pManifestParam : pManifest->parameters()) {
        parameters.insert(
                pManifestParam->id(),
                EngineEffectParameterPointer(
                        new EngineEffectParameter(pManifestParam)));
    }
    pEffect->loadEngineEffectParameters(parameters);
    return parameters;
}

} // anonymous namespace

// Tests the TrackerEffect built-in effect. These tests verify the manifest
// structure and the DSP processing logic directly, complementing the
// existing effect framework integration tests (metaknob_link_test.cpp).
class TrackerEffectTest : public MixxxTest {
  protected:
    void SetUp() override {
        MixxxTest::SetUp();
        m_engineParameters = std::make_unique<EngineParameters>(
                kTestSampleRate, kTestBufferSize);
        m_pEffect = std::make_unique<TrackerEffect>();
        m_parameters = createAndLoadParameters(m_pEffect.get());
        m_pState = std::make_unique<TrackerEffectGroupState>(*m_engineParameters);
    }

    std::unique_ptr<EngineParameters> m_engineParameters;
    std::unique_ptr<TrackerEffect> m_pEffect;
    QMap<QString, EngineEffectParameterPointer> m_parameters;
    std::unique_ptr<TrackerEffectGroupState> m_pState;
};

TEST_F(TrackerEffectTest, manifestIsValid) {
    EffectManifestPointer pManifest = TrackerEffect::getManifest();
    ASSERT_NE(nullptr, pManifest);
    EXPECT_EQ(pManifest->id(), TrackerEffect::getId());
    EXPECT_FALSE(pManifest->name().isEmpty());
    EXPECT_FALSE(pManifest->description().isEmpty());
    EXPECT_FALSE(pManifest->author().isEmpty());
    EXPECT_FALSE(pManifest->version().isEmpty());

    // The manifest should expose all nine tracker DSP parameters
    const auto& params = pManifest->parameters();
    EXPECT_EQ(params.size(), 9);

    QStringList paramIds;
    for (const auto& p : params) {
        paramIds.append(p->id());
    }
    EXPECT_TRUE(paramIds.contains("reverb"));
    EXPECT_TRUE(paramIds.contains("reverb_depth"));
    EXPECT_TRUE(paramIds.contains("reverb_delay"));
    EXPECT_TRUE(paramIds.contains("megabass"));
    EXPECT_TRUE(paramIds.contains("bass_depth"));
    EXPECT_TRUE(paramIds.contains("surround"));
    EXPECT_TRUE(paramIds.contains("surround_depth"));
    EXPECT_TRUE(paramIds.contains("surround_delay"));
    EXPECT_TRUE(paramIds.contains("noise_reduction"));
}

TEST_F(TrackerEffectTest, processChannelDisabledPassthrough) {
    // With the effect disabled, processChannel should copy input to output
    SampleBuffer inputBuffer(m_engineParameters->samplesPerBuffer());
    SampleBuffer outputBuffer(m_engineParameters->samplesPerBuffer());

    // Fill input with a test signal
    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        inputBuffer.data()[i] = 0.5f * (i % 2 == 0 ? 1.0f : -1.0f);
    }

    GroupFeatureState groupFeatures;
    m_pEffect->processChannel(
            m_pState.get(),
            inputBuffer.data(),
            outputBuffer.data(),
            *m_engineParameters,
            EffectEnableState::Disabled,
            groupFeatures);

    // When disabled, output should match input (passthrough)
    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        EXPECT_NEAR(outputBuffer.data()[i], inputBuffer.data()[i], 0.001f)
                << "Mismatch at sample" << i;
    }
}

TEST_F(TrackerEffectTest, processChannelEnabledDoesNotCrash) {
    // With the effect enabled, processing should not crash and should
    // produce finite output values
    SampleBuffer inputBuffer(m_engineParameters->samplesPerBuffer());
    SampleBuffer outputBuffer(m_engineParameters->samplesPerBuffer());

    // Fill input with a simple sine-like signal
    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        inputBuffer.data()[i] =
                0.3f * static_cast<CSAMPLE>(sin(i * 0.01));
    }

    GroupFeatureState groupFeatures;
    m_pEffect->processChannel(
            m_pState.get(),
            inputBuffer.data(),
            outputBuffer.data(),
            *m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures);

    // All output values must be finite (no NaN or Inf)
    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        EXPECT_TRUE(std::isfinite(outputBuffer.data()[i]))
                << "Non-finite output at sample" << i;
    }
}

TEST_F(TrackerEffectTest, processChannelSilenceInput) {
    // Processing silence should produce finite (and near-zero) output
    SampleBuffer inputBuffer(m_engineParameters->samplesPerBuffer());
    SampleBuffer outputBuffer(m_engineParameters->samplesPerBuffer());

    SampleUtil::clear(inputBuffer.data(), m_engineParameters->samplesPerBuffer());

    GroupFeatureState groupFeatures;
    m_pEffect->processChannel(
            m_pState.get(),
            inputBuffer.data(),
            outputBuffer.data(),
            *m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures);

    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        EXPECT_TRUE(std::isfinite(outputBuffer.data()[i]));
        EXPECT_NEAR(outputBuffer.data()[i], 0.0f, 0.1f)
                << "Non-silent output at sample" << i;
    }
}

TEST_F(TrackerEffectTest, processChannelWithReverbEnabled) {
    // Enable reverb and process - should not crash, output should be finite
    m_parameters.value("reverb")->setValue(1.0);

    SampleBuffer inputBuffer(m_engineParameters->samplesPerBuffer());
    SampleBuffer outputBuffer(m_engineParameters->samplesPerBuffer());

    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        inputBuffer.data()[i] =
                0.3f * static_cast<CSAMPLE>(sin(i * 0.01));
    }

    GroupFeatureState groupFeatures;
    m_pEffect->processChannel(
            m_pState.get(),
            inputBuffer.data(),
            outputBuffer.data(),
            *m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures);

    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        EXPECT_TRUE(std::isfinite(outputBuffer.data()[i]))
                << "Non-finite output at sample" << i;
    }
}

TEST_F(TrackerEffectTest, processChannelWithAllEffectsEnabled) {
    // Enable all effects and process - should not crash
    m_parameters.value("reverb")->setValue(1.0);
    m_parameters.value("megabass")->setValue(1.0);
    m_parameters.value("surround")->setValue(1.0);
    m_parameters.value("noise_reduction")->setValue(1.0);

    SampleBuffer inputBuffer(m_engineParameters->samplesPerBuffer());
    SampleBuffer outputBuffer(m_engineParameters->samplesPerBuffer());

    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        inputBuffer.data()[i] =
                0.3f * static_cast<CSAMPLE>(sin(i * 0.01));
    }

    GroupFeatureState groupFeatures;
    m_pEffect->processChannel(
            m_pState.get(),
            inputBuffer.data(),
            outputBuffer.data(),
            *m_engineParameters,
            EffectEnableState::Enabled,
            groupFeatures);

    for (SINT i = 0; i < m_engineParameters->samplesPerBuffer(); ++i) {
        EXPECT_TRUE(std::isfinite(outputBuffer.data()[i]))
                << "Non-finite output at sample" << i;
    }
}

TEST_F(TrackerEffectTest, stateResetDoesNotCrash) {
    // Resetting state should not crash
    m_pState->reset();
    m_pState->reset();
}

} // namespace mixxx
