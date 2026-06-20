#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Vintage Tape Delay Effect
// Simulates a tape echo machine with low-pass filtering in the feedback loop

class VintageDelayGroupState : public EffectState {
  public:
    static constexpr int kMaxDelaySeconds = 3;

    VintageDelayGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~VintageDelayGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        delay_buf.clear();
        prev_time = 0.0f;
        prev_feedback = 0.0f;
        prev_tone = 0.0f;
        prev_mix = 0.0f;
        write_position = 0;
        // Clear filter states
        lp_state[0] = 0.0f;
        lp_state[1] = 0.0f;
    };

    mixxx::SampleBuffer delay_buf;
    float prev_time;
    float prev_feedback;
    float prev_tone;
    float prev_mix;
    int write_position;

    // Low-pass filter states (for tape character)
    CSAMPLE lp_state[2];
};

class VintageDelayEffect : public EffectProcessorImpl<VintageDelayGroupState> {
  public:
    VintageDelayEffect() = default;
    ~VintageDelayEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            VintageDelayGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pTimeParameter;
    EngineEffectParameterPointer m_pFeedbackParameter;
    EngineEffectParameterPointer m_pToneParameter;
    EngineEffectParameterPointer m_pMixParameter;

    DISALLOW_COPY_AND_ASSIGN(VintageDelayEffect);
};
