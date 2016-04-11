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

    EffectManifestParameter* stereo = manifest.addParameter();
    stereo->setId("stereo");
    stereo->setName(QObject::tr("Stereo"));
    stereo->setDescription(QObject::tr("Enables/disables stereo"));
    stereo->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    stereo->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    stereo->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    stereo->setDefault(0);
    stereo->setMinimum(0);
    stereo->setMaximum(1);

    EffectManifestParameter* stages = manifest.addParameter();
    stages->setId("stages");
    stages->setName(QObject::tr("Stages"));
    stages->setDescription(QObject::tr("Sets number of stages."));
    stages->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    stages->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    stages->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    stages->setDefault(1.0);
    stages->setMinimum(1.0);
    stages->setMaximum(6.0);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("lfo_frequency");
    frequency->setName(QObject::tr("Rate"));
    frequency->setDescription(QObject::tr("Controls the speed of the low frequency oscilator."));
    frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    frequency->setDefault(0.5);
    frequency->setMinimum(0.0);
    frequency->setMaximum(5.0);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("Controls the intensity of the effect.");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefault(0.0);
    depth->setMinimum(0.0);
    depth->setMaximum(1.0);

    EffectManifestParameter* fb = manifest.addParameter();
    fb->setId("feedback");
    fb->setName(QObject::tr("Feedback"));
    fb->setDescription(QObject::tr("Controls how much of the output signal is looped"));
    fb->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    fb->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    fb->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    fb->setDefault(0.0);
    fb->setMinimum(-0.95);
    fb->setMaximum(0.95);

    EffectManifestParameter* range = manifest.addParameter();
    range->setId("range");
    range->setName(QObject::tr("Range"));
    range->setDescription(QObject::tr("Controls the frequency range across which the notches sweep."));
    range->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    range->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    range->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    range->setDefault(0.5);
    range->setMinimum(0.05);
    range->setMaximum(0.95);

    return manifest;
}

PhaserEffect::PhaserEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pStagesParameter(pEffect->getParameterById("stages")),
          m_pLFOFrequencyParameter(pEffect->getParameterById("lfo_frequency")),
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pFeedbackParameter(pEffect->getParameterById("feedback")),
          m_pRangeParameter(pEffect->getParameterById("range")),
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
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);

    CSAMPLE frequency = m_pLFOFrequencyParameter->value();
    CSAMPLE depth = m_pDepthParameter->value();
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
    CSAMPLE freqSkip = frequency * 2.0 * M_PI / sampleRate;

    int stereoCheck = m_pStereoParameter->value();
    int counter = 0;

    const int kChannels = 2;
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

        // Computing output combining the original and processed sample
        pOutput[i] = pInput[i] * (1.0 - 0.5 * depth) + left * depth * 0.5;
        pOutput[i + 1] = pInput[i + 1] * (1.0 - 0.5 * depth) + right * depth * 0.5;
    }
}
