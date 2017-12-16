#include "effects/native/tremoloeffect.h"

namespace {
//  Used to avoid gain discontinuities when changing parameters too fast
constexpr double kMaxGainIncrement = 0.001;

constexpr int kNumberOfChannels = 2;
}

// static
QString TremoloEffect::getId() {
    return "org.mixxx.effects.tremolo";
}

// static
EffectManifest TremoloEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Tremolo"));
    manifest.setShortName(QObject::tr("Tremolo"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("An amplitude modulation effect");

    EffectManifestParameter* rate = manifest.addParameter();
    rate->setId("rate");
    rate->setName(QObject::tr("Rate"));
    rate->setShortName(QObject::tr("Rate"));
    rate->setDescription(QObject::tr("Controls the rate of the effect\n"
    "4 beats - 1/8 beat if tempo is detected (decks and samplers) \n"
    "1/4 Hz - 8 Hz if no tempo is detected (mic & aux inputs, master mix)"));
    rate->setControlHint(
        EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    rate->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    rate->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    rate->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    rate->setDefault(1);
    rate->setMinimum(1.0/4);
    rate->setMaximum(8);

    EffectManifestParameter* shape = manifest.addParameter();
    shape->setId("shape");
    shape->setName(QObject::tr("Shape"));
    shape->setShortName(QObject::tr("Shape"));
    shape->setDescription(QObject::tr("Sets the length of the modulation\n"
    "10% - 90% of the effect period"));
    shape->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    shape->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    shape->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    shape->setMinimum(0.1);
    shape->setDefault(0.5);
    shape->setMaximum(0.9);

    EffectManifestParameter* waveform = manifest.addParameter();
    waveform->setId("waveform");
    waveform->setName(QObject::tr("Waveform"));
    waveform->setShortName(QObject::tr("Waveform"));
    waveform->setDescription(QObject::tr("Sets the waveform of the modulation\n"
    "Fully left - Square wave\n"
    "Fully right - Sine wave"));
    waveform->setControlHint(
        EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    waveform->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    waveform->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    waveform->setMinimum(0.005);
    waveform->setDefault(0.5);
    waveform->setMaximum(1);


    EffectManifestParameter *phase = manifest.addParameter();
    phase->setId("phase");
    phase->setName("Phase");
    phase->setShortName(QObject::tr("Phase"));
    phase->setDescription("Shift the modulation inside the period\n"
    "0 - 1 period shift");
    phase->setControlHint(
        EffectManifestParameter::ControlHint::KNOB_LINEAR);
    phase->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    phase->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    phase->setDefault(0);
    phase->setMinimum(0);
    phase->setMaximum(1);

    EffectManifestParameter* quantize = manifest.addParameter();
    quantize->setId("quantize");
    quantize->setName("Quantize");
    quantize->setShortName("Quantize");
    quantize->setDescription(
        "Round the Rate parameter to the nearest whole division of a beat.");
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
    triplet->setShortName(QObject::tr("Triplet"));
    triplet->setDescription("When the Quantize parameter is enabled, divide "
                            "the effect period by 3.");
    triplet->setControlHint(
        EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    return manifest;
}

TremoloEffect::TremoloEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pRateParameter(pEffect->getParameterById("rate")),
          m_pShapeParameter(pEffect->getParameterById("shape")),
          m_pWaveformParameter(pEffect->getParameterById("waveform")),
          m_pPhaseParameter(pEffect->getParameterById("phase")),
          m_pQuantizeParameter(pEffect->getParameterById("quantize")),
          m_pTripletParameter(pEffect->getParameterById("triplet")) {
    Q_UNUSED(manifest);
}

TremoloEffect::~TremoloEffect() {
}

void TremoloEffect::processChannel(const ChannelHandle& handle,
                                TremoloGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);

    const double shape = m_pShapeParameter->value();
    const double smooth = m_pWaveformParameter->value();

    unsigned int currentFrame = pState->currentFrame;
    double gain = pState->gain;

    const GroupFeatureState& gf = groupFeatures;

    bool quantizeEnabling = !pState->quantizeEnabled
                          && m_pQuantizeParameter->toBool();
    bool tripletDisabling = pState->tripletEnabled
                          && !m_pTripletParameter->toBool();

    if (enableState == EffectProcessor::ENABLING
     || quantizeEnabling
     || tripletDisabling) {
        if (gf.has_beat_length_sec && gf.has_beat_fraction) {
            currentFrame = gf.beat_fraction * gf.beat_length_sec * sampleRate;
        } else {
            currentFrame = 0;
        }
        gain = 0;
    }

    int framePerPeriod;
    double rate = m_pRateParameter->value();
    if (gf.has_beat_length_sec && gf.has_beat_fraction) {
        if (m_pQuantizeParameter->toBool()) {
            int divider = log2(rate);
            rate = pow(2, divider);

            if (m_pTripletParameter->toBool()) {
                rate *= 3.0;
            }
        }
        int framePerBeat = gf.beat_length_sec * sampleRate;
        framePerPeriod = framePerBeat / rate;
    } else {
        framePerPeriod = sampleRate / rate;
    }

    unsigned int phaseOffsetFrame = m_pPhaseParameter->value() * framePerPeriod;
    currentFrame = currentFrame % framePerPeriod;

    for (unsigned int i = 0; i < numSamples; i+=kNumberOfChannels) {
        unsigned int positionFrame = (currentFrame - phaseOffsetFrame);
        positionFrame = positionFrame % framePerPeriod;

        //  Relative position (0 to 1) in the period
        double position = 1.0 * positionFrame / framePerPeriod;

        //  Bend the position according to the shape parameter
        //  This maps [0 shape] to [0 0.5] and [shape 1] to [0.5 1]
        if (position < shape) {
            position = 0.5 / shape * position;
        } else {
            position = 0.5 + 0.5 * (position - shape) / (1 - shape);
        }

        //  This is where the magic happens
        //  This function gives the gain to apply for position in [0 1]
        //  Plot the function to get a grasp :
        //  From a sine to a square wave depending on the smooth parameter
        double gainTarget = 0.5 + atan(sin(2.0 * M_PI * position) / smooth)
                                   / (2 * atan(1 / smooth));

        if (gainTarget > gain + kMaxGainIncrement) {
            gain += kMaxGainIncrement;
        } else if (gainTarget < gain - kMaxGainIncrement) {
            gain -= kMaxGainIncrement;
        } else {
            gain = gainTarget;
        }

        for (int channel = 0; channel < kNumberOfChannels; channel++) {
            pOutput[i+channel] = gain * pInput[i+channel];
        }

        currentFrame++;
    }

    // Write back channel state
    pState->currentFrame = currentFrame;
    pState->gain = gain;
    pState->quantizeEnabled = m_pQuantizeParameter->toBool();
    pState->tripletEnabled = m_pTripletParameter->toBool();
}
