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
}

FlangerEffect::~FlangerEffect() {
    qDebug() << debugString() << "destroyed";
}

void FlangerEffect::process(const QString& group,
                            const CSAMPLE* pInput, CSAMPLE* pOutput,
                            const unsigned int numSamples) {
    GroupState& group_state = m_groupState[group];

    CSAMPLE lfoPeriod = m_pPeriodParameter ?
            m_pPeriodParameter->value().toDouble() : 0.0f;
    CSAMPLE lfoDepth = m_pDepthParameter ?
            m_pDepthParameter->value().toDouble() : 0.0f;
    // Unused in EngineFlanger
    CSAMPLE lfoDelay = m_pDelayParameter ?
            m_pDelayParameter->value().toDouble() : 0.0f;

    // TODO(rryan) check ranges
    // period needs to be >=0
    // delay needs to be >=0
    // depth is ???

    CSAMPLE* delayLeft = group_state.delayLeft;
    CSAMPLE* delayRight = group_state.delayRight;

    const int kChannels = 2;
    for (int i = 0; i < numSamples; i += kChannels) {
        delayLeft[group_state.delayPos] = pInput[i];
        delayRight[group_state.delayPos] = pInput[i+1];

        group_state.delayPos = (group_state.delayPos + 1) % kMaxDelay;

        group_state.time++;
        if (group_state.time > lfoPeriod) {
            group_state.time = 0;
        }

        CSAMPLE periodFraction = CSAMPLE(group_state.time) / lfoPeriod;
        CSAMPLE delay = kAverageDelayLength + kLfoAmplitude * sin(two_pi * periodFraction);

        int framePrev = (group_state.delayPos - int(delay) + kMaxDelay - 1) % kMaxDelay;
        int frameNext = (group_state.delayPos - int(delay) + kMaxDelay    ) % kMaxDelay;
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
