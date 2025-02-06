#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

struct BitCrusherGroupState : public EffectState {
    // Default accumulator to 1 so we immediately pick an input value.
    BitCrusherGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              hold_l(0),
              hold_r(0),
              accumulator(1) {
    }
    ~BitCrusherGroupState() override = default;

    CSAMPLE hold_l;
    CSAMPLE hold_r;
    // Accumulated fractions of a samplerate period.
    CSAMPLE accumulator;
};

class BitCrusherEffect : public EffectProcessorImpl<BitCrusherGroupState> {
  public:
    BitCrusherEffect() = default;
    ~BitCrusherEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            BitCrusherGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pBitDepthParameter;
    EngineEffectParameterPointer m_pDownsampleParameter;

    DISALLOW_COPY_AND_ASSIGN(BitCrusherEffect);
};
