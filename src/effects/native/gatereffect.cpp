#include "effects/native/gatereffect.h"



#include "util/sample.h"

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
    rate->setControlHint(EffectManifestParameter::ControlHint::KNOB_STEPPING);
    rate->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    rate->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    rate->appendStep(QPair<QString, double>("1", 0));
    rate->appendStep(QPair<QString, double>("1/2", 0.34));
    rate->appendStep(QPair<QString, double>("1/4", 0.67));
    rate->appendStep(QPair<QString, double>("1/8", 1));

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

    EffectManifestParameter *quantize = manifest.addParameter();
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

    EffectManifestParameter *triplet = manifest.addParameter();
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
    invert->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
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


    const auto rate  = m_pRateParameter->value();
    const auto shape  = m_pShapeParameter->value();

    // Used to divide a beat by a power of 2
    int divider = pow(2, ((int)(3*rate)));

    // Attack & Release increase with the divider factor
    double attackInc = baseAttackInc*divider;
    double releaseInc = baseReleaseInc*divider;

    // Get channel specific state variable
    unsigned int timePosition    = pState->timePosition[handle];
    unsigned int holdCounter     = pState->holdCounter[handle];
    double       gain            = pState->gain[handle];
    GaterGroupState::State state = pState->state[handle];

    // Use to keep track of the beginning of beats when playing with the rate
    unsigned int beatPeriod = 1;
    // Initialization
    if(enableState == EffectProcessor::ENABLING) {
        // Start with the gate closed
        state = GaterGroupState::IDLE;
        gain = 0;

        // Set initial time from current beat position, or 0 if no beat grid
        if(groupFeatures.has_beat_length_sec && groupFeatures.has_beat_fraction) {
            timePosition = groupFeatures.beat_fraction*groupFeatures.beat_length_sec*sampleRate;
            beatPeriod   = groupFeatures.beat_length_sec*sampleRate;
        } else {
            timePosition = 0;   
            beatPeriod   = sampleRate; // 1 second
        }
    }

    // Adjust trigger period
    unsigned int triggerPeriod;
    double period = 1-rate;
    if (groupFeatures.has_beat_length_sec) {
        // triggerPeriod is a number of beats
        if (m_pQuantizeParameter->toBool()) {
            // Divide a beat by the divider factor
            period = 1.0/divider;
            
            if (m_pTripletParameter->toBool()) {
                period /= 3.0;
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
    unsigned int triggerTime = m_pInvertParameter->toBool() ? triggerPeriod/2 : 0;
    // holdTime : number of samples spent in the Hold state
    unsigned int holdTime = std::max(0, (int)(triggerPeriod*shape - maxGain/attackInc - maxGain/releaseInc));
    

    for(unsigned int i = 0; i < numSamples; i+=2) {
         if (timePosition % triggerPeriod == triggerTime) {
            state = GaterGroupState::ATTACK;
            if(timePosition % beatPeriod == 1)
                timePosition = 0;
        }
        timePosition++;

        // Gate state machine : Idle->Attack->Hold->Release->Idle
        switch(state) {
            // Idle : Gain = 0
            case GaterGroupState::IDLE:
                gain = 0;
                break;

            // Attack : Increase gain up to maxGain, then Hold
            case GaterGroupState::ATTACK:
                gain += attackInc;
                if(gain >= maxGain) {
                    state = GaterGroupState::HOLD;
                    holdCounter = holdTime;
                }
                break;

            // Hold : Hold maxGain for holdCounter samples
            case GaterGroupState::HOLD:
                gain = maxGain;
                if(holdCounter == 0)
                    state = GaterGroupState::RELEASE;  

                holdCounter--;
                break;

            // Release : Decrease gain to 0, then Idle
            case GaterGroupState::RELEASE:
                gain -= releaseInc;
                if(gain <= 0)
                    state = GaterGroupState::IDLE;
                break;
        }

        pOutput[i]   = gain*pInput[i];
        pOutput[i+1] = gain*pInput[i+1];
    }

    // Write back channel state
    pState->timePosition[handle] = timePosition;
    pState->holdCounter[handle]  = holdCounter;
    pState->gain[handle]         = gain;
    pState->state[handle]        = state;
}
