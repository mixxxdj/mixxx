#include "effects/native/utilityeffect.h"

#include "util/sample.h"

// static
QString UtilityEffect::getId() {
    return "org.mixxx.effects.utility";
}

// static
EffectManifest UtilityEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Utility"));
    manifest.setAuthor("sqrwvzblw");
    manifest.setVersion("0.1");
    manifest.setDescription(QObject::tr(
        "Utility is a simple effect unit comprising of"
    	"a simple level/gain knob and a mute switch for"
        "muting audio of selected channel."));
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* paramA = manifest.addParameter();
    paramA->setId("knob_A");
    paramA->setName(QObject::tr("Gain"));
    paramA->setDescription(QObject::tr("Adjusts Gain."));
    paramA->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    paramA->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    paramA->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    paramA->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    paramA->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::INVERTED);
    paramA->setNeutralPointOnScale(1.0);
    paramA->setDefault(1.0);
    paramA->setMinimum(0.00);
    paramA->setMaximum(2.0);

    EffectManifestParameter* switchA = manifest.addParameter();
    switchA->setId("switch_A");
    switchA->setName(QObject::tr("Mute"));
    switchA->setDescription(QObject::tr("Mute audio passing through this effect block"));
    switchA->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    switchA->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    switchA->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    switchA->setDefault(1);
    switchA->setMinimum(0);
    switchA->setMaximum(1);

    return manifest;
}

UtilityEffect::UtilityEffect(EngineEffect* pEffect,
                                   const EffectManifest& manifest)
        : m_pKnobA(pEffect->getParameterById("knob_A")),
		  m_pSwitchA(pEffect->getParameterById("switch_A")) {
    Q_UNUSED(manifest);
}

UtilityEffect::~UtilityEffect() {
    //qDebug() << debugString() << "destroyed";
}

void UtilityEffect::rampGainCalculation(UtilityGroupState* pState,
		                                   bool mute_state,
										   CSAMPLE ramp_step,
										   const EffectProcessor::EnableState enableState ) {

	//decode enable state/mute state transitions for ramping volume properly.

	// unmute->mute state transition
	if ((pState->previous_mute_state)  && (!mute_state) ) {
		//rampgain ramps down with rampstep till zero.
		pState->ramp_state = UtilityGroupState::RAMPDOWN;
		//set previous mute state to mute
		pState->previous_mute_state = true;
	}

	// mute->unmute state transition
	if ((!pState->previous_mute_state)  && (mute_state) ) {
		//rampgain ramps up from zero to 1.0
		pState->ramp_state = UtilityGroupState::RAMPUP;
		//set previous mute state to unmute
		pState->previous_mute_state = false;
	}

	// disable->enable state transition
	if (pState->previous_enable_state == EffectProcessor::DISABLED && enableState == EffectProcessor::ENABLED) {
		//ramp gain ramps up from zero to 1.0
		pState->ramp_state = UtilityGroupState::RAMPUP;
		//set previous enable state to enable
		pState->previous_enable_state = EffectProcessor::ENABLED;
	}

	// enable->disable state transition
	if ((pState->previous_enable_state == EffectProcessor::ENABLED) && (enableState == EffectProcessor::DISABLED)) {
		//ramp gain ramps down with rampstep till zero.
		pState->ramp_state = UtilityGroupState::RAMPDOWN;
		//set previous enable state to disable
		pState->previous_enable_state = EffectProcessor::DISABLED;
	}

	// increment or decrement rampgain depending on rampstate.
	if(pState->ramp_state == UtilityGroupState::RAMPDOWN) {
		pState->ramp_gain-=ramp_step;
	}

	if(pState->ramp_state == UtilityGroupState::RAMPUP) {
		pState->ramp_gain+=ramp_step;
	}

	//add ramp gain guard to stop ramping.
	if (pState->ramp_gain > 1.0) {
		pState->ramp_gain =1.0;
		pState->ramp_state = UtilityGroupState::RAMPNO;
	}

	if (pState->ramp_gain < 0.0) {
		pState->ramp_gain =0.0;
		pState->ramp_state = UtilityGroupState::RAMPNO;
	}

}

void UtilityEffect::processChannel(const ChannelHandle& handle,
                                      UtilityGroupState* pState,
                                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                                      const unsigned int numSamples,
                                      const unsigned int sampleRate,
                                      const EffectProcessor::EnableState enableState,
                                      const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);


    CSAMPLE effect_gain = m_pKnobA ?
    		m_pKnobA->value() : 0.0;

    bool current_mute_state = m_pSwitchA->toBool();

    //if effect is disabled and ramping complete return,
    if ((enableState == EffectProcessor::DISABLED)&&(pState->ramp_state == UtilityGroupState::RAMPNO)) {
        return;
    }

	//increase and decrease ramp factor from 0.0 to 1.0 and back with this step.
	CSAMPLE ramp_step = (pState->ramp_time_in_seconds * numSamples)/(2.0*sampleRate);

	UtilityEffect::rampGainCalculation(pState, current_mute_state, ramp_step, enableState );


    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {

        // mute switch check
        if (!current_mute_state) {
        	//set previous mute state to mute
        	pState->ramp_state = UtilityGroupState::RAMPDOWN;
        }

        pState->previous_mute_state = current_mute_state;

        pOutput[i] = pInput[i] * effect_gain * pState->ramp_gain;
        pOutput[i+1] = pInput[i+1] * effect_gain * pState->ramp_gain;

    }
}
