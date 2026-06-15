// Ported from CAPS Reverb.
// This effect is GPL code.

#pragma once

#include <Reverb.h>

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"
#include "engine/filters/enginefilterbutterworth2.h"
#include "engine/filters/enginefilterbutterworth4.h"
#include "engine/filters/enginefilterbutterworth6.h"
#include "engine/filters/enginefilterbutterworth8.h"

class BandpassReverbGroupState : public EffectState {
  public:
    BandpassReverbGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              sampleRate(engineParameters.sampleRate()),
              sendPrevious(0),
               butter2(engineParameters.sampleRate(), 200.0, 2000.0),
                butter4(engineParameters.sampleRate(), 200.0, 2000.0),
                butter6(engineParameters.sampleRate(), 200.0, 2000.0),
                butter8(engineParameters.sampleRate(), 200.0, 2000.0) {
        reverb.init(sampleRate);
    }
    
    ~BandpassReverbGroupState() override = default;

    float sampleRate;
    float sendPrevious;
    MixxxPlateX2 reverb;

    EngineFilterButterworth2Band butter2;
    EngineFilterButterworth4Band butter4;
    EngineFilterButterworth6Band butter6;
    EngineFilterButterworth8Band butter8;
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
    EngineEffectParameterPointer m_pHPCutoffParameter;
    EngineEffectParameterPointer m_pLPCutoffParameter;
    EngineEffectParameterPointer m_pPostFilterParameter;
    EngineEffectParameterPointer m_pFilterOrderParameter;
    
    DISALLOW_COPY_AND_ASSIGN(BandpassReverbEffect);
};