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

// This class provides a float value that cannot be increased or decreased
// by more than a given value to avoid clicks.
// Finally I use it with only one value but i wonder if it can be useful
// somewhere else (I hear clicks when I change the period of flanger for example).
class RampedSample {
  public:
    
    inline RampedSample()
        : ramped(false),
          maxDifference(1.0f),
          initialized(false) {}
    
    virtual ~RampedSample(){};
    
    inline void setRamping(const float newMaxDifference) {
        maxDifference = newMaxDifference;
    }
    
    inline RampedSample & operator=(const float newValue) {
        if (!initialized) {
            currentValue = newValue;
            initialized = true;
        } else {
            float difference = newValue - currentValue;
            if (fabs(difference) > maxDifference) {
                currentValue += difference / fabs(difference) * maxDifference;
                ramped = true;
            } else {
                currentValue = newValue;
            }
        }
        return *this;
    }
    
    inline operator float() {
        return currentValue;
    }
    
    // TODO(jclaveau) : remove when maxDiff value is fixed
    bool ramped;
    
  private:
    float maxDifference;
    float currentValue;
    bool initialized;
};


struct PanGroupState {
    PanGroupState() {
        time = 0;
    }
    ~PanGroupState() {
    }
    unsigned int time;
    RampedSample frac;
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
    
    DISALLOW_COPY_AND_ASSIGN(PanEffect);
};

#endif /* PANEFFECT_H */
