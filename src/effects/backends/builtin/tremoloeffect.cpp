#include "effects/backends/builtin/tremoloeffect.h"

namespace {
//  Used to avoid gain discontinuities when changing parameters too fast
constexpr double kMaxGainIncrement = 0.001;
} // namespace

// static
QString TremoloEffect::getId() {
    return "org.mixxx.effects.tremolo";
}

// static
EffectManifestPointer TremoloEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Tremolo"));
    pManifest->setShortName(QObject::tr("Tremolo"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Cycles the volume up and down"));

    EffectManifestParameterPointer depth = pManifest->addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setShortName(QObject::tr("Depth"));
    depth->setDescription(QObject::tr(
            "How much the effect changes the volume"));
    depth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    depth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    depth->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    depth->setRange(0, 1, 1);

    EffectManifestParameterPointer rate = pManifest->addParameter();
    rate->setId("rate");
    rate->setName(QObject::tr("Rate"));
    rate->setShortName(QObject::tr("Rate"));
    rate->setDescription(QObject::tr(
            "Rate of the volume changes\n"
            "4 beats - 1/8 beat if tempo is detected\n"
            "1/4 Hz - 8 Hz if no tempo is detected"));
    rate->setValueScaler(
            EffectManifestParameter::ValueScaler::Logarithmic);
    rate->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    rate->setRange(1.0 / 4, 1, 8);

    EffectManifestParameterPointer width = pManifest->addParameter();
    width->setId("width");
    width->setName(QObject::tr("Width"));
    width->setShortName(QObject::tr("Width"));
    width->setDescription(QObject::tr(
            "Width of the volume peak\n"
            "10% - 90% of the effect period"));
    width->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    width->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    width->setRange(0.1, 0.5, 0.9);

    EffectManifestParameterPointer waveform = pManifest->addParameter();
    waveform->setId("waveform");
    waveform->setName(QObject::tr("Waveform"));
    waveform->setShortName(QObject::tr("Waveform"));
    waveform->setDescription(QObject::tr(
            "Shape of the volume modulation wave\n"
            "Fully left: Square wave\n"
            "Fully right: Sine wave"));
    waveform->setValueScaler(
            EffectManifestParameter::ValueScaler::Logarithmic);
    waveform->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    waveform->setRange(0.005, 0.5, 1);

    EffectManifestParameterPointer phase = pManifest->addParameter();
    phase->setId("phase");
    phase->setName(QObject::tr("Phase"));
    phase->setShortName(QObject::tr("Phase"));
    phase->setDescription(QObject::tr(
            "Shifts the position of the volume peak within the period\n"
            "Fully left: beginning of the effect period\n"
            "Fully right: end of the effect period"));
    phase->setValueScaler(
            EffectManifestParameter::ValueScaler::Linear);
    phase->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    phase->setRange(0, 0, 1);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round the Rate parameter to the nearest whole division of a beat."));
    quantize->setValueScaler(
            EffectManifestParameter::ValueScaler::Toggle);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    quantize->setRange(0, 1, 1);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplet"));
    triplet->setDescription(QObject::tr(
            "When the Quantize parameter is enabled, divide the effect period by 3."));
    triplet->setValueScaler(
            EffectManifestParameter::ValueScaler::Toggle);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    triplet->setRange(0, 0, 1);

    return pManifest;
}

void TremoloEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDepthParameter = parameters.value("depth");
    m_pRateParameter = parameters.value("rate");
    m_pWidthParameter = parameters.value("width");
    m_pWaveformParameter = parameters.value("waveform");
    m_pPhaseParameter = parameters.value("phase");
    m_pQuantizeParameter = parameters.value("quantize");
    m_pTripletParameter = parameters.value("triplet");
}

void TremoloEffect::processChannel(
        TremoloState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    const double width = m_pWidthParameter->value();
    const double smooth = m_pWaveformParameter->value();
    const double depth = m_pDepthParameter->value();

    unsigned int currentFrame = pState->currentFrame;
    double gain = pState->gain;

    const GroupFeatureState& gf = groupFeatures;

    bool quantizeEnabling = !pState->quantizeEnabled && m_pQuantizeParameter->toBool();
    bool tripletDisabling = pState->tripletEnabled && !m_pTripletParameter->toBool();

    if (enableState == EffectEnableState::Enabling || quantizeEnabling || tripletDisabling) {
        if (gf.beat_length.has_value() && gf.beat_fraction_buffer_end.has_value()) {
            currentFrame = static_cast<unsigned int>(*gf.beat_fraction_buffer_end *
                    gf.beat_length->seconds * engineParameters.sampleRate());
        } else {
            currentFrame = 0;
        }
        gain = 0;
    }

    int framePerPeriod;
    double rate = m_pRateParameter->value();
    if (gf.beat_length.has_value() && gf.beat_fraction_buffer_end.has_value()) {
        if (m_pQuantizeParameter->toBool()) {
            const auto divider = static_cast<int>(log2(rate));
            rate = pow(2, divider);

            if (m_pTripletParameter->toBool()) {
                rate *= 3.0;
            }
        }
        const auto framePerBeat = static_cast<int>(
                gf.beat_length->seconds * engineParameters.sampleRate());
        framePerPeriod = static_cast<int>(framePerBeat / rate);
    } else {
        framePerPeriod = static_cast<int>(engineParameters.sampleRate() / rate);
    }

    const auto phaseOffsetFrame = static_cast<unsigned int>(
            m_pPhaseParameter->value() * framePerPeriod);
    currentFrame = currentFrame % framePerPeriod;

    for (SINT i = 0;
            i < engineParameters.samplesPerBuffer();
            i += engineParameters.channelCount()) {
        unsigned int positionFrame = (currentFrame - phaseOffsetFrame);
        positionFrame = positionFrame % framePerPeriod;

        //  Relative position (0 to 1) in the period
        double position = static_cast<double>(positionFrame) / framePerPeriod;

        //  Bend the position according to the width parameter
        //  This maps [0 width] to [0 0.5] and [width 1] to [0.5 1]
        if (position < width) {
            position = 0.5 / width * position;
        } else {
            position = 0.5 + 0.5 * (position - width) / (1 - width);
        }

        //  This is where the magic happens
        //  This function gives the gain to apply for position in [0 1]
        //  Plot the function to get a grasp :
        //  From a sine to a square wave depending on the smooth parameter
        double gainTarget = 1.0 - (depth / 2.0) +
                (atan(sin(2.0 * M_PI * position) / smooth) /
                        (2 * atan(1 / smooth))) *
                        depth;

        if (gainTarget > gain + kMaxGainIncrement) {
            gain += kMaxGainIncrement;
        } else if (gainTarget < gain - kMaxGainIncrement) {
            gain -= kMaxGainIncrement;
        } else {
            gain = gainTarget;
        }

        for (int channel = 0; channel < engineParameters.channelCount(); channel++) {
            pOutput[i + channel] = static_cast<CSAMPLE_GAIN>(gain) * pInput[i + channel];
        }

        currentFrame++;
    }

    // Write back channel state
    pState->currentFrame = currentFrame;
    pState->gain = gain;
    pState->quantizeEnabled = m_pQuantizeParameter->toBool();
    pState->tripletEnabled = m_pTripletParameter->toBool();
}
