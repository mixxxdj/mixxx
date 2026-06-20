#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Beat Repeat / Stutter effect
// Captures a loop of audio and repeats it at various speeds
// Similar to Rekordbox's Beat Repeat or Serato's Stutter

class BeatRepeatGroupState : public EffectState {
  public:
    static constexpr int kMaxDelaySeconds = 4;

    BeatRepeatGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~BeatRepeatGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        delay_buf.clear();
        prev_enable = 0.0f;
        prev_interval = 0.0f;
        prev_pitch = 0.0f;
        write_position = 0;
        loop_start = 0;
        loop_length = 0;
        read_position = 0;
        is_recording = true;
        repeat_count = 0;
        fade_state = 0.0f;
    };

    mixxx::SampleBuffer delay_buf;
    CSAMPLE_GAIN prev_enable;
    CSAMPLE_GAIN prev_interval;
    CSAMPLE_GAIN prev_pitch;
    int write_position;
    int loop_start;
    int loop_length;
    double read_position;
    bool is_recording;
    int repeat_count;
    CSAMPLE fade_state;
};

class BeatRepeatEffect : public EffectProcessorImpl<BeatRepeatGroupState> {
  public:
    BeatRepeatEffect() = default;
    ~BeatRepeatEffect() override = default();

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            BeatRepeatGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    CSAMPLE processRepeatSample(BeatRepeatGroupState* pState,
            int loopLen,
            CSAMPLE_GAIN decay,
            CSAMPLE_GAIN gate);

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pEnableParameter;
    EngineEffectParameterPointer m_pIntervalParameter;
    EngineEffectParameterPointer m_pPitchParameter;
    EngineEffectParameterPointer m_pDecayParameter;
    EngineEffectParameterPointer m_pGateParameter;

    DISALLOW_COPY_AND_ASSIGN(BeatRepeatEffect);
};
