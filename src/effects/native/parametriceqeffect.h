#ifndef PARAMERICEQEFFECT_H
#define PARAMERICEQEFFECT_H

#include <QMap>

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

class ParametricEQEffectGroupState final {
  public:
    ParametricEQEffectGroupState();
    ~ParametricEQEffectGroupState();

    void setFilters(int sampleRate);

    QList<EngineFilterBiquad1Peaking*> m_bands;
    QList<double> m_oldGain;
    QList<double> m_oldCenter;
    QList<double> m_oldQ;

    QList<CSAMPLE*> m_pBufs;
};

class ParametricEQEffect : public PerChannelEffectProcessor<ParametricEQEffectGroupState> {
  public:
    ParametricEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~ParametricEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        ParametricEQEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    QList<EngineEffectParameter*> m_pPotGain;
    QList<EngineEffectParameter*> m_pPotQ;
    QList<EngineEffectParameter*> m_pPotCenter;


    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(ParametricEQEffect);
};

#endif // PARAMERICEQEFFECT_H
