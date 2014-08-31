#ifndef BESSEL8LVMIXEQEFFECT_H
#define BESSEL8LVMIXEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbessel8.h"
#include "engine/enginefilterdelay.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

// constant to calculate the group delay from the low pass corner
// mean value of a set of fid_calc_delay() calls for different corners
const double kGroupDelay1Hz = 0.5067964223;
// kDelayOffset is required to match short delays.
const double kDelayOffset = 0.2;
const double kMaxCornerFreq = 14212;
const int kMaxDelay = 3300; // allows a 30 Hz filter at 97346;

class Bessel8LVMixEQEffectGroupState {
  public:
    Bessel8LVMixEQEffectGroupState();
    virtual ~Bessel8LVMixEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterBessel8Low* m_low1;
    EngineFilterBessel8Low* m_low2;
    EngineFilterDelay<kMaxDelay>* m_delay2;
    EngineFilterDelay<kMaxDelay>* m_delay3;

    double old_low;
    double old_mid;
    double old_high;

    unsigned int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class Bessel8LVMixEQEffect : public GroupEffectProcessor<Bessel8LVMixEQEffectGroupState> {
  public:
    Bessel8LVMixEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~Bessel8LVMixEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      Bessel8LVMixEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
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

    DISALLOW_COPY_AND_ASSIGN(Bessel8LVMixEQEffect);
};

#endif /* BESSEL8LVMIXEQEFFECT_H */
