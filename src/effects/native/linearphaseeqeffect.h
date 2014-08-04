#ifndef LINEARPHASEEQEFFECT_H
#define LINEARPHASEEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth8.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class LinearPhaseEQEffectGroupState {
  public:
    LinearPhaseEQEffectGroupState();
    virtual ~LinearPhaseEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterButterworth8High* low;
    EngineFilterButterworth8High* high;

    double old_low;
    double old_mid;
    double old_high;

    unsigned int m_windowSize;
    int m_processPos;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pHighBuf;
    CSAMPLE* m_pWindow;
    CSAMPLE* m_pLastIn;
    CSAMPLE* m_pProcessBuf;
    CSAMPLE* m_pLastProcessBuf;
};

class LinearPhaseEQEffect : public GroupEffectProcessor<LinearPhaseEQEffectGroupState> {
  public:
    LinearPhaseEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~LinearPhaseEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      LinearPhaseEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    ControlObjectSlave* m_pLoFreqCorner;
    ControlObjectSlave* m_pHiFreqCorner;

    int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    DISALLOW_COPY_AND_ASSIGN(LinearPhaseEQEffect);
};

#endif /* LINEARPHASEEQEFFECT_H */
