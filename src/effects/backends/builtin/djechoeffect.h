#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

class DJEchoGroupState : public EffectState {
  public:
    // 4 seconds max delay
    static constexpr int kMaxDelaySeconds = 4;

    DJEchoGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~DJEchoGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        delay_buf.clear();
        prev_send = 0.0f;
        prev_feedback = 0.0f;
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong = 0;
        // Clear filter states
        feedback_filter_state[0] = 0.0f;
        feedback_filter_state[1] = 0.0f;
        feedback_lp_state[0] = 0.0f;
        feedback_lp_state[1] = 0.0f;
    };

    mixxx::SampleBuffer delay_buf;
    CSAMPLE_GAIN prev_send;
    CSAMPLE_GAIN prev_feedback;
    int prev_delay_samples;
    int write_position;
    int ping_pong;

    // Feedback filter states (for warmth and hi-cut)
    CSAMPLE_GAIN feedback_filter_state[2];
    CSAMPLE_GAIN feedback_lp_state[2];
};

class DJEchoEffect : public EffectProcessorImpl<DJEchoGroupState> {
  public:
    DJEchoEffect() = default;
    ~DJEchoEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            DJEchoGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pDelayParameter;
    EngineEffectParameterPointer m_pSendParameter;
    EngineEffectParameterPointer m_pFeedbackParameter;
    EngineEffectParameterPointer m_pPingPongParameter;
    EngineEffectParameterPointer m_pQuantizeParameter;
    EngineEffectParameterPointer m_pTripletParameter;
    EngineEffectParameterPointer m_pHiHatParameter;
    EngineEffectParameterPointer m_pWarmthParameter;

    DISALLOW_COPY_AND_ASSIGN(DJEchoEffect);
};
