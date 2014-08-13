#ifndef GRAPHICEQEFFECT_H
#define GRAPHICEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class GraphicEQEffectGroupState {
  public:
    GraphicEQEffectGroupState();
    virtual ~GraphicEQEffectGroupState();

    void setFilters(int sampleRate);

    QList<EngineFilterBiquad1Band*> m_bands;
    QList<double> m_oldMid;
    QList<CSAMPLE*> m_pBandBuf;

  private:
    float m_centerFrequencies[8];
};

class GraphicEQEffect : public GroupEffectProcessor<GraphicEQEffectGroupState> {
  public:
    GraphicEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~GraphicEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      GraphicEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    QList<EngineEffectParameter*> m_pPotMid;
    int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(GraphicEQEffect);
};

#endif // GRAPHICEQEFFECT_H
