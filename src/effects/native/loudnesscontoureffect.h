#ifndef LOUDNESSCONTOUREFFECT_H
#define LOUDNESSCONTOUREFFECT_H

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"
#include "util/memory.h"

class LoudnessContourEffectGroupState final {
  public:
    LoudnessContourEffectGroupState();
    ~LoudnessContourEffectGroupState();

    void setFilters(int sampleRate, double gain);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_low;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_high;
    CSAMPLE* m_pBuf;
    double m_oldGainKnob;
    double m_oldLoudness;
    double m_oldGain;
    double m_oldFilterGainDb;
    bool m_oldUseGain;
    unsigned int m_oldSampleRate;
};

class LoudnessContourEffect
        : public PerChannelEffectProcessor<LoudnessContourEffectGroupState> {
  public:
    LoudnessContourEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    ~LoudnessContourEffect() override;

    static QString getId();
    static EffectManifest getManifest();

    void setFilters(int sampleRate);

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        LoudnessContourEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

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
