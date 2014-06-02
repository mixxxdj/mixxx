#ifndef EQDEFAULT_H
#define EQDEFAULT_H

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

class EQDefaultGroupState {
public:
    EQDefaultGroupState();
    ~EQDefaultGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterButterworth8Low* low;
    EngineFilterButterworth8Band* band;
    EngineFilterButterworth8High* high;

    double old_low;
    double old_mid;
    double old_high;
    double old_dry;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class EQDefault : public GroupEffectProcessor<EQDefaultGroupState> {
  public:
    EQDefault(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~EQDefault();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      EQDefaultGroupState* pState,
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

    DISALLOW_COPY_AND_ASSIGN(EQDefault);
};

#endif /* EQDEFAULT_H */
