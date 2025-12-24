#pragma once
#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class EngineFilterMoogLadder4Low;
class EngineFilterMoogLadder4High;

class MoogLadder4FilterGroupState : public EffectState {
  public:
    MoogLadder4FilterGroupState(const mixxx::EngineParameters& engineParameters);
    ~MoogLadder4FilterGroupState() override;

    void setFilters(mixxx::audio::SampleRate sampleRate, double lowFreq, double highFreq);

    CSAMPLE* m_pBuf;
    EngineFilterMoogLadder4Low* m_pLowFilter;
    EngineFilterMoogLadder4High* m_pHighFilter;

    double m_loFreq;
    double m_resonance;
    double m_hiFreq;
    double m_samplerate;
};

class MoogLadder4FilterEffect : public EffectProcessorImpl<MoogLadder4FilterGroupState> {
  public:
    MoogLadder4FilterEffect() = default;
    ~MoogLadder4FilterEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            MoogLadder4FilterGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pLPF;
    EngineEffectParameterPointer m_pResonance;
    EngineEffectParameterPointer m_pHPF;

    DISALLOW_COPY_AND_ASSIGN(MoogLadder4FilterEffect);
};
