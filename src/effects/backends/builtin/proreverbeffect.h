#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Professional reverb effect inspired by Rekordbox/Serato reverb
// Uses a multi-tap delay line with feedback for rich, musical reverb
// Supports multiple reverb modes: Hall, Room, Plate, Spring

class ProReverbGroupState : public EffectState {
  public:
    // 5 seconds max reverb tail
    static constexpr int kMaxDelaySeconds = 5;
    static constexpr int kNumTaps = 8;

    ProReverbGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~ProReverbGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        delay_buf.clear();
        prev_send = 0.0f;
        prev_feedback = 0.0f;
        prev_size = 0.0f;
        write_position = 0;
        for (int i = 0; i < kNumTaps; ++i) {
            tap_gains[i] = 0.0f;
            tap_delays[i] = 0;
        }
        // Diffusion filters (all-pass)
        diff_state[0] = 0.0f;
        diff_state[1] = 0.0f;
        diff_state[2] = 0.0f;
        diff_state[3] = 0.0f;
        diff_state[4] = 0.0f;
    };

    mixxx::SampleBuffer delay_buf;
    CSAMPLE_GAIN prev_send;
    CSAMPLE_GAIN prev_feedback;
    CSAMPLE_GAIN prev_size;
    int write_position;
    CSAMPLE tap_gains[kNumTaps];
    int tap_delays[kNumTaps];
    CSAMPLE diff_state[5];
};

class ProReverbEffect : public EffectProcessorImpl<ProReverbGroupState> {
  public:
    ProReverbEffect() = default;
    ~ProReverbEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ProReverbGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    void updateTapDelays(ProReverbGroupState* pState,
            const mixxx::EngineParameters& engineParameters,
            int size_param);

    EngineEffectParameterPointer m_pSendParameter;
    EngineEffectParameterPointer m_pFeedbackParameter;
    EngineEffectParameterPointer m_pSizeParameter;
    EngineEffectParameterPointer m_pDampingParameter;
    EngineEffectParameterPointer m_pModeParameter;
    EngineEffectParameterPointer m_pDryWetParameter;

    DISALLOW_COPY_AND_ASSIGN(ProReverbEffect);
};
