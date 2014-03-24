#ifndef FLANGEREFFECT_H
#define FLANGEREFFECT_H

#include <QMap>

#include "defs.h"
#include "util.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

struct FlangerGroupState {
    FlangerGroupState()
            : delayPos(0),
              time(0) {
        SampleUtil::applyGain(delayLeft, 0, MAX_BUFFER_LEN);
        SampleUtil::applyGain(delayRight, 0, MAX_BUFFER_LEN);
    }
    CSAMPLE delayRight[MAX_BUFFER_LEN];
    CSAMPLE delayLeft[MAX_BUFFER_LEN];
    unsigned int delayPos;
    unsigned int time;
};

class FlangerEffect : public GroupEffectProcessor<FlangerGroupState> {
  public:
    FlangerEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~FlangerEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      FlangerGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPeriodParameter;
    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pDelayParameter;

    DISALLOW_COPY_AND_ASSIGN(FlangerEffect);
};

#endif /* FLANGEREFFECT_H */
