#ifndef GOASLICEREFFECT_H
#define GOASLICEREFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "sampleutil.h"
#include "util.h"

struct GoaSlicerGroupState {
    GoaSlicerGroupState()
            // TODO(XXX) 44100 should be changed to real sample rate
            // https://bugs.launchpad.net/mixxx/+bug/1208816.
            : length(2048),
              slope(512) {
    }

    unsigned int length;
    unsigned int slope;
    unsigned int period;
};

class GoaSlicerEffect : public GroupEffectProcessor<GoaSlicerGroupState> {
  public:
    GoaSlicerEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~GoaSlicerEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      GoaSlicerGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatures);

  private:
    
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pLengthParameter;
    EngineEffectParameter* m_pSlopeParameter;
    EngineEffectParameter* m_pPeriodParameter;

    DISALLOW_COPY_AND_ASSIGN(GoaSlicerEffect);
};

#endif /* GOASLICEREFFECT_H */
