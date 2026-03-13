// Ported from CAPS Reverb.
// This effect is GPL code.

#pragma once

#include <Reverb.h>

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"
#include "engine/filters/simplebandpass.h"

class BandpassReverbGroupState : public EffectState {
  public:
    BandpassReverbGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              sampleRate(engineParameters.sampleRate()),
              sendPrevious(0) {
        reverb.init(sampleRate);
    }
    
    ~BandpassReverbGroupState() override = default;

    float sampleRate;
    float sendPrevious;
    MixxxPlateX2 reverb;
    SimpleBandPass bandPass;
};

class BandpassReverbEffect : public EffectProcessorImpl<BandpassReverbGroupState> {
  public:
    BandpassReverbEffect() = default;
    ~BandpassReverbEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            BandpassReverbGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pDecayParameter;
    EngineEffectParameterPointer m_pBandWidthParameter;
    EngineEffectParameterPointer m_pDampingParameter;
    EngineEffectParameterPointer m_pSendParameter;
    EngineEffectParameterPointer m_pBPFreqParameter;
    EngineEffectParameterPointer m_pBPQParameter;

    DISALLOW_COPY_AND_ASSIGN(BandpassReverbEffect);
};