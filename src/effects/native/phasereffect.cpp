#include "effects/native/phasereffect.h"

#include <QDebug>

#include "util/math.h"

const unsigned int updateCoef = 32;

// static
QString PhaserEffect::getId() {
    return "org.mixxx.effects.phaser";
}

// static
EffectManifest PhaserEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Phaser"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
                "A more complex sound effect obtained by mixing the input signal"
                " with a copy passed through a series of all-pass filters."));
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* period = manifest.addParameter();
    period->setId("lfo_period");
    period->setName(QObject::tr("Period"));
    period->setDescription(QObject::tr("Controls the period of the LFO (low frequency oscillator)\n"
        "1/4 - 4 beats rounded to 1/2 beat if tempo is detected (decks and samplers) \n"
        "1/4 - 4 seconds if no tempo is detected (mic & aux inputs, master mix)"));
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    period->setMinimum(0.0);
    period->setMaximum(4.0);
    period->setDefault(1.0);

    EffectManifestParameter* fb = manifest.addParameter();
    fb->setId("feedback");
    fb->setName(QObject::tr("Feedback"));
    fb->setDescription(QObject::tr("Controls how much of the output signal is looped"));
    fb->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    fb->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    fb->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    fb->setMinimum(-1.0);
    fb->setMaximum(1.0);
    fb->setDefault(0.0);

    EffectManifestParameter* range = manifest.addParameter();
    range->setId("range");
    range->setName(QObject::tr("Range"));
    range->setDescription(QObject::tr("Controls the frequency range across which the notches sweep."));
    range->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    range->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    range->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    range->setMinimum(0.05);
    range->setMaximum(1.0);
    range->setDefault(1.0);

    EffectManifestParameter* stages = manifest.addParameter();
    stages->setId("stages");
    stages->setName(QObject::tr("Stages"));
    stages->setDescription(QObject::tr("Sets number of stages."));
    stages->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    stages->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    stages->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    stages->setMinimum(1.0);
    stages->setMaximum(6.0);
    stages->setDefault(3.5);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("Controls the intensity of the effect.");
    depth->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    depth->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    depth->setMinimum(0.5);
    depth->setMaximum(1.0);
    depth->setDefault(0.5);

    EffectManifestParameter* triplet = manifest.addParameter();
    triplet->setId("triplet");
    triplet->setName("Triplets");
    triplet->setDescription("Divide rounded 1/2 beats of the Period parameter by 3.");
    triplet->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    EffectManifestParameter* stereo = manifest.addParameter();
    stereo->setId("stereo");
    stereo->setName(QObject::tr("Stereo"));
    stereo->setDescription(QObject::tr("Enables/disables stereo"));
    stereo->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    stereo->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    stereo->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    stereo->setMinimum(0);
    stereo->setMaximum(1);
    stereo->setDefault(0);

    return manifest;
}

PhaserEffect::PhaserEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pStagesParameter(pEffect->getParameterById("stages")),
          m_pLFOPeriodParameter(pEffect->getParameterById("lfo_period")),
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pFeedbackParameter(pEffect->getParameterById("feedback")),
          m_pRangeParameter(pEffect->getParameterById("range")),
          m_pTripletParameter(pEffect->getParameterById("triplet")),
          m_pStereoParameter(pEffect->getParameterById("stereo")) {
    Q_UNUSED(manifest);
}

PhaserEffect::~PhaserEffect() {
    //qDebug() << debugString() << "destroyed";
}

void PhaserEffect::processChannel(const ChannelHandle& handle,
                                  PhaserGroupState* pState,
                                  const CSAMPLE* pInput, CSAMPLE* pOutput,
                                  const unsigned int numSamples,
                                  const unsigned int sampleRate,
                                  const EffectProcessor::EnableState enableState,
                                  const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);

    // TODO: remove assumption of stereo signal
    const int kChannels = 2;

    if (enableState == EffectProcessor::ENABLING) {
        pState->init();
    }

    CSAMPLE depth = 0;
    if (enableState != EffectProcessor::DISABLING) {
        depth = m_pDepthParameter->value();
    }

    double periodParameter = m_pLFOPeriodParameter->value();
    double periodSamples;
    if (groupFeatures.has_beat_length_sec) {
        // periodParameter is a number of beats
        periodParameter = std::max(roundToFraction(periodParameter, 2.0), 1/4.0);
        if (m_pTripletParameter->toBool()) {
            periodParameter /= 3.0;
        }
        periodSamples = periodParameter * groupFeatures.beat_length_sec * sampleRate;
    } else {
        // periodParameter is a number of seconds
        periodSamples = std::max(periodParameter, 1/4.0) * sampleRate;
    }
    // freqSkip is used to calculate the phase independently for each channel,
    // so do not multiply periodSamples by the number of channels.
    CSAMPLE freqSkip = 1.0 / periodSamples * 2.0 * M_PI;

    CSAMPLE feedback = m_pFeedbackParameter->value();
    CSAMPLE range = m_pRangeParameter->value();
    int stages = 2 * m_pStagesParameter->value();

    CSAMPLE* oldInLeft = pState->oldInLeft;
    CSAMPLE* oldOutLeft = pState->oldOutLeft;
    CSAMPLE* oldInRight = pState->oldInRight;
    CSAMPLE* oldOutRight = pState->oldOutRight;

    // Using two sets of coefficients for left and right channel
    CSAMPLE filterCoefLeft = 0;
    CSAMPLE filterCoefRight = 0;

    CSAMPLE left = 0, right = 0;

    CSAMPLE_GAIN oldDepth = pState->oldDepth;
    const CSAMPLE_GAIN depthDelta = (depth - oldDepth)
            / CSAMPLE_GAIN(numSamples / 2);
    const CSAMPLE_GAIN depthStart = oldDepth + depthDelta;

    int stereoCheck = m_pStereoParameter->value();
    int counter = 0;

    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        left = pInput[i] + tanh(left * feedback);
        right = pInput[i + 1] + tanh(right * feedback);

        // For stereo enabled, the channels are out of phase
        pState->leftPhase = fmodf(pState->leftPhase + freqSkip, 2.0 * M_PI);
        pState->rightPhase = fmodf(pState->rightPhase + freqSkip + M_PI * stereoCheck, 2.0 * M_PI);

        // Updating filter coefficients once every 'updateCoef' samples to avoid
        // extra computing
        if ((counter++) % updateCoef == 0) {
                CSAMPLE delayLeft = 0.5 + 0.5 * sin(pState->leftPhase);
                CSAMPLE delayRight = 0.5 + 0.5 * sin(pState->rightPhase);

                // Coefficient computing based on the following:
                // https://ccrma.stanford.edu/~jos/pasp/Classic_Virtual_Analog_Phase.html
                CSAMPLE wLeft = range * delayLeft;
                CSAMPLE wRight = range * delayRight;

                CSAMPLE tanwLeft = tanh(wLeft / 2);
                CSAMPLE tanwRight = tanh(wRight / 2);

                filterCoefLeft = (1.0 - tanwLeft) / (1.0 + tanwLeft);
                filterCoefRight = (1.0 - tanwRight) / (1.0 + tanwRight);
        }

        left = processSample(left, oldInLeft, oldOutLeft, filterCoefLeft, stages);
        right = processSample(right, oldInRight, oldOutRight, filterCoefRight, stages);

        const CSAMPLE_GAIN depth = depthStart + depthDelta * (i / kChannels);

        // Computing output combining the original and processed sample
        pOutput[i] = pInput[i] * (1.0 - 0.5 * depth) + left * depth * 0.5;
        pOutput[i + 1] = pInput[i + 1] * (1.0 - 0.5 * depth) + right * depth * 0.5;
    }

    pState->oldDepth = depth;
}
