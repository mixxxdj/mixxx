#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Professional DJ Flanger
#LFO - modulated delay line with feedback, based on Calf DSP flanger algorithms

class FlangerGroupState : public EffectState {
  public:
    static constexpr int kMaxDelaySamples = 4410; // 100ms at 44.1kHz

    FlangerGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~FlangerGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        m_sampleRate = engineParameters.sampleRate();
        delay_buf = mixxx::SampleBuffer(kMaxDelaySamples);
    };

    void clear() {
        delay_buf.clear();
        prev_rate = 0.0f;
        prev_depth = 0.0f;
        prev_feedback = 0.0f;
        prev_stereo = false;
        write_position = 0;
        lfo_phase = 0.0f;
        prev_delay_samples = 0;
    };

    mixxx::SampleBuffer delay_buf;
    int m_sampleRate;
    float prev_rate;
    float prev_depth;
    float prev_feedback;
    bool prev_stereo;
    int write_position;
    float lfo_phase;
    int prev_delay_samples;
};

class ProFlangerEffect : public EffectProcessorImpl<FlangerGroupState> {
  public:
    ProFlangerEffect() = default;
    ~ProFlangerEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            FlangerGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pRateParameter;
    EngineEffectParameterPointer m_pDepthParameter;
    EngineEffectParameterPointer m_pFeedbackParameter;
    EngineEffectParameterPointer m_pStereoParameter;

    DISALLOW_COPY_AND_ASSIGN(ProFlangerEffect);
};
