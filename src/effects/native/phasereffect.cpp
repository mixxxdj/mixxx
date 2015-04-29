#include "util/math.h"
#include <QDebug>

#include "effects/native/phasereffect.h"

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
                "with a copy passed through a series of all-pass filters."));
    
    EffectManifestParameter* stages = manifest.addParameter();
    stages->setId("stages");
    stages->setName(QObject::tr("Stages"));
    stages->setDescription("Sets number of stages.");
    stages->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    stages->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    stages->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    stages->setDefault(1.0);
    stages->setMinimum(1.0);
    stages->setMaximum(12.0);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("lfo_frequency");
    frequency->setName(QObject::tr("Rate"));
    frequency->setDescription("Controls the speed of the effect.");   
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
    fb->setDescription("Controls how much of the output signal is looped");
    fb->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    fb->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    fb->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    fb->setDefault(0.0);
    fb->setMinimum(-1.0);
    fb->setMaximum(1.0);

    EffectManifestParameter* range = manifest.addParameter();
    range->setId("range");
    range->setName(QObject::tr("Range"));
    range->setDescription("Controls the frequency range across which the notches sweep.");
    range->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    range->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    range->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    range->setDefault(0.5);
    range->setMinimum(0.0);
    range->setMaximum(1.0);

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

    CSAMPLE filterCoefLeft = 0;
    CSAMPLE filterCoefRight = 0;

    CSAMPLE left = 0, right = 0;
    CSAMPLE leftPhase, rightPhase;
    CSAMPLE freqSkip = frequency * 2.0 * M_PI / sampleRate;

    int stereoCheck = m_pStereoParameter->value();
    int counter = 0;

    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {

        pState->time++;
        left = pInput[i] + left * feedback; 
        right = pInput[i + 1] + right * feedback;

        leftPhase = fmodf(freqSkip * pState->time, 2.0 * M_PI);
        rightPhase = fmodf(freqSkip * pState->time + M_PI * stereoCheck, 2.0 * M_PI);

        if ((counter++) % updateCoef == 0) {
                CSAMPLE delayLeft = 0.5 + 0.5 * sin(leftPhase);
                CSAMPLE delayRight = 0.5 + 0.5 * sin(rightPhase);
                
                CSAMPLE wLeft = range * delayLeft;
                CSAMPLE wRight = range * delayRight;

                CSAMPLE tanwLeft = tanh(wLeft / 2);
                CSAMPLE tanwRight = tanh(wRight / 2);

                filterCoefLeft = (1.0 - tanwLeft) / (1.0 + tanwLeft);
                filterCoefRight = (1.0 - tanwRight) / (1.0 + tanwRight);

        }

        for (int j = 0; j < stages; j++) {
            oldOutLeft[j] = (filterCoefLeft * left) +
                (filterCoefLeft * oldOutLeft[j]) - 
                oldInLeft[j];
            oldInLeft[j] = left;
            left = oldOutLeft[j];

            oldOutRight[j] = (filterCoefRight * right) +
                (filterCoefRight * oldOutRight[j]) - 
                oldInRight[j];
            oldInRight[j] = right;
            right = oldOutRight[j];
        }
        pOutput[i] = pInput[i] * (1.0 - 0.5 * depth) + left * depth * 0.5;
        pOutput[i + 1] = pInput[i + 1] * (1.0 - 0.5 * depth) + right * depth * 0.5;
    }
}
