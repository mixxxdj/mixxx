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

struct PanSquarySinusoid {
    PanSquarySinusoid() {
        lastFract = -1;
    }
    ~PanSquarySinusoid() {
    }
    CSAMPLE lastFract;
};

struct PanGroupState {
    PanGroupState() {
        time = 0;
    }
    ~PanGroupState() {
    }
    unsigned int time;
};

class RampedSample {
  public:
    RampedSample();
    inline RampedSample(const float newMaxDifference){
        setRamping(newMaxDifference);
    }
    
    virtual ~RampedSample();
    inline void setRamping(const float newMaxDifference){
        maxDifference = newMaxDifference;
    }
    
    inline RampedSample & operator=(const float &newValue) {
        CSAMPLE difference = currentValue - newValue;
        if (difference > maxDifference)
            currentValue = maxDifference;
        else
            currentValue = newValue;
        
        return *this;
    }
    
    inline RampedSample & operator-(float diff) {
        CSAMPLE newValue = currentValue - diff;
        CSAMPLE difference = currentValue - newValue;
        if (difference > maxDifference)
            currentValue = maxDifference;
        else
            currentValue = newValue;
        
        return *this;
    }
    
    inline RampedSample & operator*(float times) {
        CSAMPLE newValue = times * currentValue;
        
        CSAMPLE difference = currentValue - newValue;
        if (difference > maxDifference)
            currentValue = maxDifference;
        else
            currentValue = newValue;
        
        return *this;
    }
    
    inline operator float() {
        return value();
    }
    
    inline RampedSample & operator=(const RampedSample &that){
        CSAMPLE difference = currentValue - that.value();
        if (difference > maxDifference)
            currentValue = maxDifference;
        else
            currentValue = that.value();
        
        return *this;
    }
    
    inline CSAMPLE value() const {
        return currentValue;
    }
    
  private:
    float maxDifference;
    CSAMPLE currentValue;
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
