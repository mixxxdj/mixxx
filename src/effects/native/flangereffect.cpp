#include <QtDebug>

#include "effects/native/flangereffect.h"

#include "mathstuff.h"

const unsigned int kMaxDelay = 5000;
const unsigned int kLfoAmplitude = 240;
const unsigned int kAverageDelayLength = 250;

// static
QString FlangerEffect::getId() {
    return "org.mixxx.effects.flanger";
}

// static
EffectManifest FlangerEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Flanger"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("TODO");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefault(0.0);
    depth->setMinimum(0.0);
    depth->setMaximum(1.0);

    EffectManifestParameter* delay = manifest.addParameter();
    delay->setId("delay");
    delay->setName(QObject::tr("Delay"));
    delay->setDescription("TODO");
    delay->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    delay->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    delay->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    delay->setDefault(50.0);
    delay->setMinimum(50.0);
    delay->setMaximum(10000.0);

    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("TODO");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setDefault(50000.0);
    period->setMinimum(50000.0);
    period->setMaximum(2000000.0);

    return manifest;
}

FlangerEffect::FlangerEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pDelayParameter(pEffect->getParameterById("delay")) {
    Q_UNUSED(manifest);
}

FlangerEffect::~FlangerEffect() {
    qDebug() << debugString() << "destroyed";
}

void FlangerEffect::processGroup(const QString& group,
                                 FlangerGroupState* pState,
                                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                                 const unsigned int numSamples) {
    Q_UNUSED(group);
    CSAMPLE lfoPeriod = m_pPeriodParameter ?
            m_pPeriodParameter->value().toDouble() : 0.0f;
    CSAMPLE lfoDepth = m_pDepthParameter ?
            m_pDepthParameter->value().toDouble() : 0.0f;
    // Unused in EngineFlanger
    // CSAMPLE lfoDelay = m_pDelayParameter ?
    //         m_pDelayParameter->value().toDouble() : 0.0f;

    // TODO(rryan) check ranges
    // period needs to be >=0
    // delay needs to be >=0
    // depth is ???

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        delayLeft[pState->delayPos] = pInput[i];
        delayRight[pState->delayPos] = pInput[i+1];

        pState->delayPos = (pState->delayPos + 1) % kMaxDelay;

        pState->time++;
        if (pState->time > lfoPeriod) {
            pState->time = 0;
        }

        CSAMPLE periodFraction = CSAMPLE(pState->time) / lfoPeriod;
        CSAMPLE delay = kAverageDelayLength + kLfoAmplitude * sin(two_pi * periodFraction);

        int framePrev = (pState->delayPos - int(delay) + kMaxDelay - 1) % kMaxDelay;
        int frameNext = (pState->delayPos - int(delay) + kMaxDelay    ) % kMaxDelay;
        CSAMPLE prevLeft = delayLeft[framePrev];
        CSAMPLE nextLeft = delayLeft[frameNext];

        CSAMPLE prevRight = delayRight[framePrev];
        CSAMPLE nextRight = delayRight[frameNext];

        CSAMPLE frac = delay - floorf(delay);
        CSAMPLE delayedSampleLeft = prevLeft + frac * (nextLeft - prevLeft);
        CSAMPLE delayedSampleRight = prevRight + frac * (nextRight - prevRight);

        pOutput[i] = pInput[i] + lfoDepth * delayedSampleLeft;
        pOutput[i+1] = pInput[i+1] + lfoDepth * delayedSampleRight;
    }
}
