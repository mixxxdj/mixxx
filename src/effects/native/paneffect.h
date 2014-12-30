#ifndef PANEFFECT_H
#define PANEFFECT_H

#include <QMap>

#include "util.h"
#include "util/defs.h"
#include "util/types.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

struct PanGroupState {
    PanGroupState() {
        time = 0;
    }
    ~PanGroupState() {
    }
    unsigned int time;
};

class PanEffect : public GroupEffectProcessor<PanGroupState> {
  public:
    PanEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~PanEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      PanGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatures);

  private:
    
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pStrengthParameter;
    EngineEffectParameter* m_pPeriodParameter;
    EngineEffectParameter* m_pRampingParameter;
    
    CSAMPLE oldFrac;
    
    DISALLOW_COPY_AND_ASSIGN(PanEffect);
};

#endif /* PANEFFECT_H */
