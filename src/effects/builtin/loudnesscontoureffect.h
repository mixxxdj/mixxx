#ifndef LOUDNESSCONTOUREFFECT_H
#define LOUDNESSCONTOUREFFECT_H

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"
#include "util/memory.h"

class LoudnessContourEffectGroupState final : public EffectState {
  public:
    LoudnessContourEffectGroupState(const mixxx::EngineParameters& bufferParameters);
    ~LoudnessContourEffectGroupState();

    void setFilters(int sampleRate, double gain);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_low;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_high;
    CSAMPLE* m_pBuf;
    double m_oldGainKnob;
    double m_oldLoudness;
    CSAMPLE_GAIN m_oldGain;
    double m_oldFilterGainDb;
    bool m_oldUseGain;
    mixxx::audio::SampleRate m_oldSampleRate;
};

class LoudnessContourEffect
        : public EffectProcessorImpl<LoudnessContourEffectGroupState> {
  public:
    LoudnessContourEffect(EngineEffect* pEffect);
    ~LoudnessContourEffect() override;

    static QString getId();
    static EffectManifestPointer getManifest();

    void setFilters(int sampleRate);

    void processChannel(const ChannelHandle& handle,
                        LoudnessContourEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatureState) override;

  private:
    LoudnessContourEffect(const LoudnessContourEffect&) = delete;
    void operator=(const LoudnessContourEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pLoudness;
    EngineEffectParameter* m_pUseGain;
};

#endif // LOUDNESSCONTOUREFFECT_H
