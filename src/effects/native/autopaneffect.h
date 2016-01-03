#ifndef AUTOPANEFFECT_H
#define AUTOPANEFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterpansingle.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

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

    inline void setRampingThreshold(const float newMaxDifference) {
        maxDifference = newMaxDifference;
    }

    inline void setWithRampingApplied(const float newValue) {
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
                ramped = false;
            }
        }
    }

    inline operator float() {
        return currentValue;
    }

    bool ramped;

  private:
    float maxDifference;
    float currentValue;
    bool initialized;
};

static const int panMaxDelay = 3300; // allows a 30 Hz filter at 97346;
// static const int panMaxDelay = 50000; // high for debug;

struct PanGroupState {
    PanGroupState() {
        time = 0;
        delay = new EngineFilterPanSingle<panMaxDelay>();
        m_pDelayBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
        m_dPreviousPeriod = -1.0;
    }
    ~PanGroupState() {
        SampleUtil::free(m_pDelayBuf);
    }
    unsigned int time;
    RampedSample frac;
    EngineFilterPanSingle<panMaxDelay>* delay;
    CSAMPLE* m_pDelayBuf;
    double m_dPreviousPeriod;
};

class AutoPanEffect : public PerChannelEffectProcessor<PanGroupState> {
  public:
    AutoPanEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~AutoPanEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                      PanGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatures);

    double computeLawCoefficient(double position);

  private:

    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pSmoothingParameter;
    EngineEffectParameter* m_pPeriodUnitParameter;
    EngineEffectParameter* m_pPeriodParameter;
    EngineEffectParameter* m_pWidthParameter;

    DISALLOW_COPY_AND_ASSIGN(AutoPanEffect);
};

#endif /* AUTOPANEFFECT_H */
