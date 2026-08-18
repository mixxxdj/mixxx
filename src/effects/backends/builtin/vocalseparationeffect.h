#pragma once

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/samplebuffer.h"
#include "util/types.h"

#include <memory>

class EngineFilterBiquad1Peaking;
class EngineFilterBiquad1High;
class EngineFilterBiquad1Low;

struct VocalSeparationGroupState : public EffectState {
    VocalSeparationGroupState(const mixxx::EngineParameters& engineParameters);
    ~VocalSeparationGroupState() override;

    void setFilters(mixxx::audio::SampleRate sampleRate, 
                    double intensity,
                    double centerFreq);

    mixxx::SampleBuffer m_tempBuffer;
    
    // Multi-band filters for vocal extraction
    std::unique_ptr<EngineFilterBiquad1Peaking> m_pVocalEnhancer1;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_pVocalEnhancer2;
    std::unique_ptr<EngineFilterBiquad1High> m_pHighPass;
    std::unique_ptr<EngineFilterBiquad1Low> m_pLowPass;
    
    double m_oldIntensity;
    double m_oldCenterFreq;
    
    // Previous intensity for ramping

    CSAMPLE m_previousIntensity;
};

class VocalSeparationEffect : public EffectProcessorImpl<VocalSeparationGroupState> {
  public:
    VocalSeparationEffect() = default;
    ~VocalSeparationEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            VocalSeparationGroupState* pState,
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
    EngineEffectParameterPointer m_pCenterFreq;
    EngineEffectParameterPointer m_pStereoWidth;

    DISALLOW_COPY_AND_ASSIGN(VocalSeparationEffect);
};
