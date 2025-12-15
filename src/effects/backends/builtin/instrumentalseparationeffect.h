#pragma once

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class EngineFilterBiquad1Peaking;
class EngineFilterBiquad1High;
class EngineFilterBiquad1Low;
class EngineFilterBiquad1Band;

struct InstrumentalSeparationGroupState : public EffectState {
    InstrumentalSeparationGroupState(const mixxx::EngineParameters& engineParameters);
    ~InstrumentalSeparationGroupState() override;

    void setFilters(mixxx::audio::SampleRate sampleRate, 
                    double intensity,
                    double bassBoost,
                    double highBoost);

    mixxx::SampleBuffer m_tempBuffer;
    
    // Multi-band filters for instrumental extraction
    EngineFilterBiquad1Peaking* m_pBassEnhancer;
    EngineFilterBiquad1Peaking* m_pMidCut;
    EngineFilterBiquad1Peaking* m_pHighEnhancer;
    EngineFilterBiquad1Band* m_pVocalNotch1;
    EngineFilterBiquad1Band* m_pVocalNotch2;
    
    double m_oldIntensity;
    double m_oldBassBoost;
    double m_oldHighBoost;
};

class InstrumentalSeparationEffect : public EffectProcessorImpl<InstrumentalSeparationGroupState> {
  public:
    InstrumentalSeparationEffect() = default;
    ~InstrumentalSeparationEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            InstrumentalSeparationGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pIntensity;
    EngineEffectParameterPointer m_pBassBoost;
    EngineEffectParameterPointer m_pHighBoost;

    DISALLOW_COPY_AND_ASSIGN(InstrumentalSeparationEffect);
};
