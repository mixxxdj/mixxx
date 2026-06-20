#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Tape Stop effect
// Simulates a tape machine slowing down to a stop
// Creates a pitch-down effect with optional feedback

class TapeStopGroupState : public EffectState {
  public:
    static constexpr int kMaxDelaySeconds = 2;

    TapeStopGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~TapeStopGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        delay_buf.clear();
        prev_enable = 0.0f;
        prev_speed = 0.0f;
        write_position = 0;
        read_position = 0.0;
        speed = 1.0;
        is_stopping = false;
        stop_start_pos = 0;
    };

    mixxx::SampleBuffer delay_buf;
    CSAMPLE_GAIN prev_enable;
    CSAMPLE_GAIN prev_speed;
    int write_position;
    double read_position;
    double speed;
    bool is_stopping;
    int stop_start_pos;
};

class TapeStopEffect : public EffectProcessorImpl<TapeStopGroupState> {
  public:
    TapeStopEffect() = default;
    ~TapeStopEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            TapeStopGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pEnableParameter;
    EngineEffectParameterPointer m_pSpeedParameter;
    EngineEffectParameterPointer m_pDurationParameter;

    DISALLOW_COPY_AND_ASSIGN(TapeStopEffect);
};
