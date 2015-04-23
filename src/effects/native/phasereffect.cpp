#include <QDebug>
#include "util/math.h"
#include "effects/native/phasereffect.h"
#include <stdlib.h>

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
    manifest.setDescription(QObject::tr("Phaser filter"));
    
    EffectManifestParameter* stages = manifest.addParameter();
    stages->setId("stages");
    stages->setName(QObject::tr("Stages"));
    stages->setDescription("Sets number of stages.");
    stages->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    stages->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    stages->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    stages->setDefault(2.0);
    stages->setMinimum(2.0);
    stages->setMaximum(24.0);

    EffectManifestParameter* lfo_frequency = manifest.addParameter();
    lfo_frequency->setId("lfo_frequency");
    lfo_frequency->setName(QObject::tr("LFO Frequency"));
    lfo_frequency->setDescription("Controls LFO Frequency.");
    lfo_frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    lfo_frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    lfo_frequency->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    lfo_frequency->setDefault(0.4);
    lfo_frequency->setMinimum(0.0);
    lfo_frequency->setMaximum(4.0);

    EffectManifestParameter* lfo_startphase = manifest.addParameter();
    lfo_startphase->setId("lfo_startphase");
    lfo_startphase->setName(QObject::tr("LFO Start Phase"));
    lfo_startphase->setDescription("Sets starting phase.");
    lfo_startphase->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    lfo_startphase->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    lfo_startphase->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    lfo_startphase->setDefault(0.0);
    lfo_startphase->setMinimum(0.0);
    lfo_startphase->setMaximum(5.0);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("Controls depth.");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefault(100.0);
    depth->setMinimum(0.0);
    depth->setMaximum(255);

    return manifest;
}

PhaserEffect::PhaserEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest) 
        : m_pStagesParameter(pEffect->getParameterById("stages")),
          m_pLFOFrequencyParameter(pEffect->getParameterById("lfo_frequency")),
          m_pLFOStartPhaseParameter(pEffect->getParameterById("lfo_startphase")),
          m_pDepthParameter(pEffect->getParameterById("depth")) {
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

    CSAMPLE lfoFrequency = m_pLFOFrequencyParameter->value();
    CSAMPLE lfoStartPhase = m_pLFOStartPhaseParameter->value();
    int depth = m_pDepthParameter->value();
    int stages = m_pStagesParameter->value();
    
    CSAMPLE* oldLeft = pState->oldLeft;
    CSAMPLE* oldRight = pState->oldRight;

    CSAMPLE* filterLeft = pState->filterCoefLeft;
    CSAMPLE* filterRight = pState->filterCoefRight;

    CSAMPLE leftOut = 0, rightOut = 0;
    CSAMPLE feedback = 0.5; 
    CSAMPLE leftPhase = lfoStartPhase, rightPhase = leftPhase + M_PI; 
    CSAMPLE lfoSkip = lfoFrequency * 2 * M_PI / sampleRate;
    
    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        leftOut = pInput[i] + leftOut * feedback; 
        rightOut = pInput[i + 1] + rightOut * feedback;

        CSAMPLE delayLeft = (sin(leftPhase) + 1) / 2;
        CSAMPLE delayRight = (sin(rightPhase) + 1) / 2;

        leftPhase += lfoSkip;
        rightPhase += lfoSkip;

        if (leftPhase >= 2.0 * M_PI) {
            leftPhase -= 2.0 * M_PI;
        }

        if (rightPhase >= 2.0 * M_PI) {
            rightPhase -= 2.0 * M_PI;
        }

        for (int j = 0; j < stages; j++) {
            filterLeft[j] = (1 - delayLeft) / (1 + delayLeft);            
            filterRight[j] = (1 - delayRight) / (1 + delayRight);
        }

        for (int j = 0; j < stages; j++) {
            CSAMPLE tmpLeft = oldLeft[j]; 
            CSAMPLE tmpRight = oldRight[j];

            oldLeft[j] = filterLeft[j] * tmpLeft + leftOut;
            oldRight[j] = filterRight[j] * tmpRight + rightOut;

            leftOut = tmpLeft - filterLeft[j] * oldLeft[j];
            rightOut = tmpRight - filterRight[j] * oldRight[j];
        }

        pOutput[i] = pInput[i] + leftOut;// * depth;
        pOutput[i + 1] = pInput[i + 1] + rightOut;// * depth;
    }
}
