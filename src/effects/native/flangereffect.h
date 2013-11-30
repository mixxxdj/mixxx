#ifndef FLANGEREFFECT_H
#define FLANGEREFFECT_H

#include <QMap>

#include "util.h"
#include "effects/effect.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/native/nativebackend.h"
#include "effects/effectprocessor.h"

const unsigned int kMaxDelay = 5000;
const unsigned int kLfoAmplitude = 240;
const unsigned int kAverageDelayLength = 250;

class FlangerEffect : public NativeEffect {
  public:
    FlangerEffect() { }
    virtual ~FlangerEffect() { }

    QString getId() const;
    EffectManifest getManifest() const;
    EffectInstantiator getInstantiator() const {
        return FlangerEffect::create;
    }
    static EffectPointer create(EffectsBackend* pBackend,
                                const EffectManifest& manifest);
};

struct FlangerState {
    CSAMPLE delayBuffer[kMaxDelay];
    unsigned int delayPos;
    unsigned int time;
};

class FlangerEffectProcessor : public EffectProcessor {
  public:
    FlangerEffectProcessor(const EffectManifest& manifest);
    virtual ~FlangerEffectProcessor();

    // See effectprocessor.h
    void initialize(EngineEffect* pEffect);

    // See effectprocessor.h
    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return "FlangerEffectProcessor";
    }
    FlangerState* getStateForGroup(const QString& group);

    EngineEffectParameter* m_periodParameter;
    EngineEffectParameter* m_depthParameter;
    EngineEffectParameter* m_delayParameter;

    QMap<QString, FlangerState*> m_flangerStates;

    DISALLOW_COPY_AND_ASSIGN(FlangerEffectProcessor);
};


#endif /* FLANGEREFFECT_H */
