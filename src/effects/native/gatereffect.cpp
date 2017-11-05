#include "effects/native/gatereffect.h"

namespace {
// Gain when the gate is open, higher than 1 to still have a decent level
constexpr double kMaxGain        = 1.5;
// Attack slope
constexpr double kBaseAttackInc  = 0.001;
// Release slope
constexpr double kBaseReleaseInc = 0.0005;
}

// static
QString GaterEffect::getId() {
    return "org.mixxx.effects.gater";
}

// static
EffectManifest GaterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Gater"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("");

    EffectManifestParameter* rate = manifest.addParameter();
    rate->setId("rate");
    rate->setName(QObject::tr("Rate"));
    rate->setDescription(QObject::tr(""));
    rate->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    rate->setDefault(0.5);
    rate->setMinimum(0);
    rate->setMaximum(1);

    EffectManifestParameter* shape = manifest.addParameter();
    shape->setId("shape");
    shape->setName(QObject::tr("Shape"));
    shape->setDescription(QObject::tr(""));
    shape->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    shape->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    shape->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    shape->setMinimum(0.1);
    shape->setDefault(0.5);
    shape->setMaximum(0.9);

    EffectManifestParameter* quantize = manifest.addParameter();
    quantize->setId("quantize");
    quantize->setName("Quantize");
    quantize->setShortName("Quantize");
    quantize->setDescription(
        "Round the Time parameter to the nearest 1/4 beat.");
    quantize->setControlHint(
        EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    quantize->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    quantize->setDefault(1);
    quantize->setMinimum(0);
    quantize->setMaximum(1);

    EffectManifestParameter* triplet = manifest.addParameter();
    triplet->setId("triplet");
    triplet->setName("Triplets");
    triplet->setDescription("When the Quantize parameter is enabled, divide "
                            "rounded 1/4 beats of Time parameter by 3.");
    triplet->setControlHint(
        EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    EffectManifestParameter *invert = manifest.addParameter();
    invert->setId("invert");
    invert->setName("Invert");
    invert->setDescription("");
    invert->setControlHint(
        EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    invert->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    invert->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    invert->setDefault(0);
    invert->setMinimum(0);
    invert->setMaximum(1);

    return manifest;
}

GaterEffect::GaterEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pRateParameter(pEffect->getParameterById("rate")),
          m_pShapeParameter(pEffect->getParameterById("shape")),
          m_pQuantizeParameter(pEffect->getParameterById("quantize")),
          m_pTripletParameter(pEffect->getParameterById("triplet")),
          m_pInvertParameter(pEffect->getParameterById("invert")) 
{
    Q_UNUSED(manifest);
}

GaterEffect::~GaterEffect() {
}

void GaterEffect::processChannel(const ChannelHandle& handle,
                                GaterGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) { 
                                    
    Q_UNUSED(handle);
                                    
    const double rate  = m_pRateParameter->value();
    const double shape  = m_pShapeParameter->value();

    // Used to divide a beat by a power of 2
    int divider = pow(2, floor(3*rate));

    // Attack & Release increase with the divider factor
    double attackInc = kBaseAttackInc*divider;
    double releaseInc = kBaseReleaseInc*divider;

    // Get channel specific state variable
    unsigned int timePosition    = pState->timePosition;
    unsigned int holdCounter     = pState->holdCounter;
    double       gain            = pState->gain;
    GaterGroupState::State state = pState->state;

    // Use to keep track of the beginning of beats when playing with the rate
    unsigned int beatPeriod = 1;
    if (enableState == EffectProcessor::ENABLING) {        
        // Start with the gate closed
        state = GaterGroupState::IDLE;
        gain = 0;
        currentFrame = 0;   
        beatPeriod   = sampleRate; // 1 second
    }

    // Set period and current frame from beatgrid, 
    if (groupFeatures.has_beat_length_sec && groupFeatures.has_beat_fraction) {
        beatPeriod   = groupFeatures.beat_length_sec*sampleRate;
        currentFrame = groupFeatures.beat_fraction*beatPeriod;
    }

    // Adjust trigger period
    unsigned int triggerPeriod;
    double period = 1-rate;
    if (groupFeatures.has_beat_length_sec) {
        // triggerPeriod is a number of beats
        if (m_pQuantizeParameter->toBool()) {
            period = 1.0/divider;
            
            if (m_pTripletParameter->toBool()) {
                period /= 3.0/4;
            }
        } else if (period < 1 / 8.0) {
            period = 1 / 8.0;
        }
        triggerPeriod = period * groupFeatures.beat_length_sec * sampleRate;
    } else {
        // triggerPeriod is a number of seconds
        period = std::max(period, 1 / 8.0);
        triggerPeriod = period * sampleRate;
    }

    // triggerTime : offset for the gate trigger
    unsigned int triggerTime;
    if (m_pInvertParameter->toBool()) {
        triggerTime = triggerPeriod/2;
    } else {
        triggerTime = 0;
    }

    // holdTime : number of samples spent in the Hold state
    unsigned int holdTime;
    double attackTime  = kMaxGain/attackInc;
    double releaseTime = kMaxGain/releaseInc;
    holdTime = std::max(0.0, floor(triggerPeriod*shape-attackTime-releaseTime));
    
    for (unsigned int i = 0; i < numSamples; i+=2) {
         if (timePosition % triggerPeriod == triggerTime) {
            state = GaterGroupState::ATTACK;
            if (timePosition % beatPeriod == 1) {
                timePosition = 0;
            }
        }
        timePosition++;

        // Gate state machine : Idle->Attack->Hold->Release->Idle
        switch (state) {
            // Idle : Gain = 0
            case GaterGroupState::IDLE:
                gain = 0;
                break;

            // Attack : Increase gain up to maxGain, then Hold
            case GaterGroupState::ATTACK:
                gain += attackInc;
                if(gain >= kMaxGain) {
                    state = GaterGroupState::HOLD;
                    holdCounter = holdTime;
                }
                break;

            // Hold : Hold maxGain for holdCounter samples
            case GaterGroupState::HOLD:
                gain = kMaxGain;
                if(holdCounter == 0) {
                    state = GaterGroupState::RELEASE;  
                }

                holdCounter--;
                break;

            // Release : Decrease gain to 0, then Idle
            case GaterGroupState::RELEASE:
                gain -= releaseInc;
                if(gain <= 0) {
                    state = GaterGroupState::IDLE;
                }
                break;
        }

        pOutput[i]   = gain*pInput[i];
        pOutput[i+1] = gain*pInput[i+1];
    }

    // Write back channel state
    pState->timePosition = timePosition;
    pState->holdCounter  = holdCounter;
    pState->gain         = gain;
    pState->state        = state;
}
