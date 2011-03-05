#ifndef FLANGEREFFECT_H
#define FLANGEREFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/native/nativebackend.h"

const unsigned int kMaxDelay = 5000;
const unsigned int kLfoAmplitude = 240;
const unsigned int kAverageDelayLength = 250;

struct FlangerState {
    CSAMPLE delayBuffer[kMaxDelay];
    unsigned int delayPos;
    unsigned int time;
};

class FlangerEffect : public Effect {
    Q_OBJECT
  public:
    FlangerEffect(NativeBackend* pBackend, EffectManifestPointer pManifest);
    virtual ~FlangerEffect();

    static EffectManifestPointer getEffectManifest();

    // See effect.h
    void process(const QString channelId,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return "FlangerEffect";
    }
    FlangerState* getStateForChannel(const QString channelId);

    EffectParameterPointer m_periodParameter;
    EffectParameterPointer m_depthParameter;
    EffectParameterPointer m_delayParameter;

    QMap<QString, FlangerState*> m_flangerStates;

};


#endif /* FLANGEREFFECT_H */
