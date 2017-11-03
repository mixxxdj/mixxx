#ifndef GATEREFFECT_H
#define GATEREFFECT_H



#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

#include <iostream>
#include <cmath>
#include <fstream>

struct GaterGroupState {
    enum State {IDLE, ATTACK, HOLD, RELEASE};
    ChannelHandleMap<State> state;
    ChannelHandleMap<double> gain;
    ChannelHandleMap<unsigned int> timePosition;
    ChannelHandleMap<unsigned int> holdCounter;
};

class GaterEffect : public PerChannelEffectProcessor<GaterGroupState> {
  public:
    GaterEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~GaterEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        GaterGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter *m_pRateParameter;
    EngineEffectParameter *m_pShapeParameter;

    // TODO : Not accessible from the UI, thus not tested yet
    EngineEffectParameter *m_pQuantizeParameter;
    EngineEffectParameter *m_pTripletParameter;
    EngineEffectParameter *m_pInvertParameter;

    // Fixed parameters
    // Gain when the gate is open, higher than 1 to still have a decent level
    double maxGain    = 1.5; 
    // Attack slope
    double baseAttackInc  = 0.001;
    // Release slope
    double baseReleaseInc = 0.0005;
    
    DISALLOW_COPY_AND_ASSIGN(GaterEffect);
};

#endif /* GATEREFFECT_H */
