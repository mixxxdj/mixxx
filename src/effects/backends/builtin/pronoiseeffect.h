#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"

// Professional noise generator for DJ transitions
// Supports white noise, pink noise, and filtered noise
// Tempo-synced fade in/out
// Inspired by Rekordbox's Noise effect

class ProNoiseGroupState : public EffectState {
  public:
    ProNoiseGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        clear();
    }
    ~ProNoiseGroupState() override = default;

    void clear() {
        // Pink noise filter state (Voss-McCartney algorithm)
        pink_state[0] = 0.0f;
        pink_state[1] = 0.0f;
        pink_state[2] = 0.0f;
        pink_state[3] = 0.0f;
        pink_state[4] = 0.0f;
        pink_state[5] = 0.0f;
        pink_state[6] = 0.0f;
        pink_running_sum = 0.0f;
        pink_index = 0;

        // Filter state for bandpass
        filter_x1 = 0.0f;
        filter_x2 = 0.0f;
        filter_y1 = 0.0f;
        filter_y2 = 0.0f;

        prev_send = 0.0f;
        prev_color = 0.0f;
        prev_bandwidth = 0.0f;
    };

    // Pink noise state
    CSAMPLE pink_state[7];
    CSAMPLE pink_running_sum;
    int pink_index;

    // Bandpass filter state
    CSAMPLE filter_x1;
    CSAMPLE filter_x2;
    CSAMPLE filter_y1;
    CSAMPLE filter_y2;

    // Smoothed parameters
    CSAMPLE prev_send;
    CSAMPLE prev_color;
    CSAMPLE prev_bandwidth;
};

class ProNoiseEffect : public EffectProcessorImpl<ProNoiseGroupState> {
  public:
    ProNoiseEffect() = default;
    ~ProNoiseEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ProNoiseGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    // Generate white noise
    CSAMPLE generateWhiteNoise();

    // Generate pink noise (1/f spectrum)
    CSAMPLE generatePinkNoise(ProNoiseGroupState* pState);

    EngineEffectParameterPointer m_pSendParameter;
    EngineEffectParameterPointer m_pColorParameter;
    EngineEffectParameterPointer m_pBandwidthParameter;
    EngineEffectParameterPointer m_pModeParameter;

    DISALLOW_COPY_AND_ASSIGN(ProNoiseEffect);
};
