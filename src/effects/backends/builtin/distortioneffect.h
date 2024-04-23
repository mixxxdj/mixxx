#pragma once

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/sample.h"
#include "util/types.h"

class DistortionGroupState : public EffectState {
  public:
    DistortionGroupState(const mixxx::EngineParameters& engineParameters);
    ~DistortionGroupState() override = default;

    CSAMPLE_GAIN m_driveGain;
    CSAMPLE m_crossfadeParameter;
    double m_samplerate;

    CSAMPLE m_previousMakeUpGain;
    CSAMPLE m_previousNormalizationGain;
};

class DistortionEffect : public EffectProcessorImpl<DistortionGroupState> {
  public:
    DistortionEffect() = default;
    ~DistortionEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            DistortionGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    enum Mode {
        SoftClipping = 0,
        HardClipping = 1,
    };

    struct SoftClippingParameters;
    struct HardClippingParameters;

    template<typename ModeParams>
    void processDistortion(CSAMPLE driveParam,
            DistortionGroupState* pState,
            CSAMPLE* pOutput,
            const CSAMPLE* pInput,
            const mixxx::EngineParameters& engineParameters) {
        SINT numSamples = engineParameters.samplesPerBuffer();

        // Normalize input
        pState->m_previousNormalizationGain =
                SampleUtil::copyWithRampingNormalization(pOutput,
                        pInput,
                        pState->m_previousNormalizationGain,
                        ModeParams::normalizationLevel,
                        numSamples);

        // Apply drive gain
        CSAMPLE_GAIN driveGain = 1 + driveParam * ModeParams::maxDriveGain;
        SampleUtil::copyWithRampingGain(
                pOutput, pInput, pState->m_driveGain, driveGain, numSamples);

        // Waveshape
        for (SINT i = 0;
                i < engineParameters.samplesPerBuffer();
                i += engineParameters.channelCount()) {
            for (int channel = 0; channel < engineParameters.channelCount(); ++channel) {
                pOutput[i + channel] = ModeParams::process(pOutput[i + channel]);
            }
        }

        // Volume compensation
        CSAMPLE pInputRMS = SampleUtil::rms(pInput, numSamples);
        CSAMPLE pOutputRMS = SampleUtil::rms(pOutput, numSamples);
        CSAMPLE_GAIN gain = pOutputRMS == CSAMPLE_ZERO
                ? 1
                : pInputRMS / pOutputRMS;

        SampleUtil::applyRampingGain(pOutput,
                pState->m_previousMakeUpGain,
                gain,
                engineParameters.samplesPerBuffer());

        pState->m_previousMakeUpGain = gain;

        // Crossfade
        CSAMPLE crossfadeParam = math_min(driveParam / ModeParams::crossfadeEndParam, 1.f);
        SampleUtil::applyRampingGain(pOutput,
                pState->m_crossfadeParameter,
                crossfadeParam,
                numSamples);
        SampleUtil::addWithRampingGain(pOutput,
                pInput,
                1 - pState->m_crossfadeParameter,
                1 - crossfadeParam,
                numSamples);

        pState->m_driveGain = driveGain;
        pState->m_crossfadeParameter = crossfadeParam;
    }

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pMode;
    EngineEffectParameterPointer m_pDrive;

    DISALLOW_COPY_AND_ASSIGN(DistortionEffect);
};
