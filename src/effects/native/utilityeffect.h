#ifndef UTILITYEFFECT_H
#define UTILITYEFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/types.h"

struct UtilityGroupState {
    // Default accumulator to 1 so we immediately pick an input value.
    UtilityGroupState()
            : ramp_state(RAMPNO),
			  previous_enable_state(EffectProcessor::DISABLED),
			  previous_mute_state(false),
			  ramp_time_in_seconds(0.8),
			  ramp_factor(0.0),
			  ramp_gain(1.0),
              hold_l(0),
              hold_r(0) {
    }
    enum RampState {
        RAMPNO = 0x00,
		RAMPUP = 0x01,
		RAMPDOWN = 0x02
    }ramp_state;
    const CSAMPLE ramp_time_in_seconds;
    CSAMPLE ramp_factor;
    CSAMPLE ramp_gain;
    EffectProcessor::EnableState previous_enable_state;
    bool previous_mute_state;

};

class UtilityEffect : public PerChannelEffectProcessor<UtilityGroupState> {
  public:
    UtilityEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~UtilityEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        UtilityGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

    void rampGainCalculation(UtilityGroupState* pState,
                        bool mute_state,
						 CSAMPLE ramp_step,
						 const EffectProcessor::EnableState enableState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pKnobA;
    EngineEffectParameter* m_pSwitchA;


    DISALLOW_COPY_AND_ASSIGN(UtilityEffect);
};

#endif /* UTILITYEFFECT_H */
