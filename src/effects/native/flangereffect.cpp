// THIS HAS TO BE THE FIRST INCLUDE!!! --kain88 (April 2013)
// http://stackoverflow.com/a/6563891
#include "util/math.h"
#include <QtDebug>

#include "effects/native/flangereffect.h"

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
    manifest.setDescription(QObject::tr(
        "A simple modulation effect, created by taking the input signal "
        "and mixing it with a delayed, pitch modulated copy of itself."));

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

    EffectManifestParameter* delay = manifest.addParameter();
    delay->setId("delay");
    delay->setName(QObject::tr("Delay"));
    delay->setDescription("Sets the value for the delay length.");
    delay->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    delay->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    delay->setDefault(50.0);
    delay->setMinimum(50.0);
    delay->setMaximum(10000.0);

    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("Controls the speed of the effect.");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
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
    //qDebug() << debugString() << "destroyed";
}

void FlangerEffect::processGroup(const QString& group,
                                 FlangerGroupState* pState,
                                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                                 const unsigned int numSamples,
                                 const unsigned int sampleRate,
                                 const EffectProcessor::EnableState enableState,
                                 const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    CSAMPLE lfoPeriod = m_pPeriodParameter->value();
    CSAMPLE lfoDepth = m_pDepthParameter->value();
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
        CSAMPLE delay = kAverageDelayLength + kLfoAmplitude * sin(M_PI * 2.0f * periodFraction);

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
