#ifndef FLANGEREFFECT_H
#define FLANGEREFFECT_H

#include <QMap>

#include "defs.h"
#include "util.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

class FlangerEffect : public EffectProcessor {
  public:
    FlangerEffect(const EffectManifest& manifest);
    virtual ~FlangerEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void initialize(EngineEffect* pEffect);

    // See effectprocessor.h
    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPeriodParameter;
    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pDelayParameter;

    struct GroupState {
        GroupState()
                : delayPos(0),
                  time(0) {
            SampleUtil::applyGain(delayBuffer, 0, MAX_BUFFER_LEN);
        }
        CSAMPLE delayBuffer[MAX_BUFFER_LEN];
        unsigned int delayPos;
        unsigned int time;
    };
    QMap<QString, GroupState> m_groupState;

    DISALLOW_COPY_AND_ASSIGN(FlangerEffect);
};

#endif /* FLANGEREFFECT_H */
