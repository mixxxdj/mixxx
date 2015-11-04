#ifndef BUTTERWORTHEQEFFECT_H
#define BUTTERWORTHEQEFFECT_H

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

class ButterworthEQEffectGroupState {
  public:
    ButterworthEQEffectGroupState();
    virtual ~ButterworthEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterButterworth8Low* low;
    EngineFilterButterworth8Band* band;
    EngineFilterButterworth8High* high;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class Butterworth8EQEffect : public GroupEffectProcessor<ButterworthEQEffectGroupState> {
  public:
    Butterworth8EQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~Butterworth8EQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      ButterworthEQEffectGroupState* pState,
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

    DISALLOW_COPY_AND_ASSIGN(Butterworth8EQEffect);
};

#endif /* BUTTERWORTHEQEFFECT_H */
