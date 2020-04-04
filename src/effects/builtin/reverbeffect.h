// Ported from CAPS Reverb.
// This effect is GPL code.

#pragma once

#include <QMap>

#include <Reverb.h>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class ReverbGroupState : public EffectState {
  public:
    ReverbGroupState(const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          sendPrevious(0) {
    }

    void engineParametersChanged(const mixxx::EngineParameters& bufferParameters) {
        sampleRate = bufferParameters.sampleRate();
        sendPrevious = 0;
    }

    float sampleRate;
    float sendPrevious;
    MixxxPlateX2 reverb{};
};

class ReverbEffect : public EffectProcessorImpl<ReverbGroupState> {
  public:
    ReverbEffect() {};
    virtual ~ReverbEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ReverbGroupState* pState,
            const CSAMPLE* pInput, CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
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

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};
