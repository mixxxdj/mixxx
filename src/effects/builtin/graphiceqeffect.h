#ifndef GRAPHICEQEFFECT_H
#define GRAPHICEQEFFECT_H

#include <QMap>

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

class GraphicEQEffectGroupState : public EffectState {
  public:
    GraphicEQEffectGroupState(const mixxx::EngineParameters& bufferParameters);
    virtual ~GraphicEQEffectGroupState();

    void setFilters(int sampleRate);

    EngineFilterBiquad1LowShelving* m_low;
    QList<EngineFilterBiquad1Peaking*> m_bands;
    EngineFilterBiquad1HighShelving* m_high;
    QList<CSAMPLE*> m_pBufs;
    QList<double> m_oldMid;
    double m_oldLow;
    double m_oldHigh;
    float m_centerFrequencies[8];
};

class GraphicEQEffect : public EffectProcessorImpl<GraphicEQEffectGroupState> {
  public:
    GraphicEQEffect(EngineEffect* pEffect);
    virtual ~GraphicEQEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
                        GraphicEQEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    QList<EngineEffectParameter*> m_pPotMid;
    EngineEffectParameter* m_pPotHigh;
    mixxx::audio::SampleRate m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(GraphicEQEffect);
};

#endif // GRAPHICEQEFFECT_H
