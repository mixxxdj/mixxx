#ifndef BIQUADSTATICEQEFFECT_H
#define BIQUADSTATICEQEFFECT_H

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

class BiquadStaticEQEffectGroupState {
  public:
    BiquadStaticEQEffectGroupState();
    virtual ~BiquadStaticEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    QList<EngineFilterBiquad1Band*> band;
    QList<double> old_mid;
    QList<CSAMPLE*> m_pBandBuf;
};

class BiquadStaticEQEffect : public GroupEffectProcessor<BiquadStaticEQEffectGroupState> {
  public:
    BiquadStaticEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~BiquadStaticEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      BiquadStaticEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    QList<EngineEffectParameter*> m_pPotMid;

    DISALLOW_COPY_AND_ASSIGN(BiquadStaticEQEffect);
};

#endif // BIQUADHSTATICEQEFFECT_H
