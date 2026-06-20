#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"

// Professional multi-mode filter effect
// Supports LPF, HPF, BPF, Notch, and peaking modes
// High resonance control for dramatic sweeps
// Inspired by Rekordbox/Serato filter quality

class ProFilterGroupState : public EffectState {
  public:
    ProFilterGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        clear();
    }
    ~ProFilterGroupState() override = default;

    void clear() {
        // Biquad filter states (stereo)
        for (int ch = 0; ch < 2; ++ch) {
            x1[ch] = 0.0f;
            x2[ch] = 0.0f;
            y1[ch] = 0.0f;
            y2[ch] = 0.0f;
        }
        // Smoothed parameter values
        prev_cutoff = 0.0f;
        prev_resonance = 0.0f;
        prev_gain = 0.0f;
    };

    // Biquad filter state variables
    CSAMPLE x1[2]; // Input history
    CSAMPLE x2[2];
    CSAMPLE y1[2]; // Output history
    CSAMPLE y2[2];

    // Smoothed parameters
    CSAMPLE prev_cutoff;
    CSAMPLE prev_resonance;
    CSAMPLE prev_gain;
};

class ProFilterEffect : public EffectProcessorImpl<ProFilterGroupState> {
  public:
    ProFilterEffect() = default;
    ~ProFilterEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ProFilterGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    // Compute biquad coefficients for different filter modes
    void computeBiquadCoefficients(
            double cutoff,
            double resonance,
            double gain,
            double* b0,
            double* b1,
            double* b2,
            double* a1,
            double* a2,
            int mode,
            double sampleRate);

    EngineEffectParameterPointer m_pCutoffParameter;
    EngineEffectParameterPointer m_pResonanceParameter;
    EngineEffectParameterPointer m_pModeParameter;
    EngineEffectParameterPointer m_pGainParameter;

    DISALLOW_COPY_AND_ASSIGN(ProFilterEffect);
};
