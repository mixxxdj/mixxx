#include <QtDebug>

#include "effects/native/flangereffect.h"

#include "mathstuff.h"
#include "sampleutil.h"

// static
QString FlangerProcessor::getId() {
    return "org.mixxx.effects.flanger";
}

// static
EffectManifest FlangerProcessor::getManifest() {
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

FlangerProcessor::FlangerProcessor(const EffectManifest& manifest)
        : m_pPeriodParameter(NULL),
          m_pDepthParameter(NULL),
          m_pDelayParameter(NULL) {
}

FlangerProcessor::~FlangerProcessor() {
    qDebug() << debugString() << "destroyed";

    QMutableMapIterator<QString, FlangerState*> it(m_flangerStates);

    while (it.hasNext()) {
        it.next();
        FlangerState* pState = it.value();
        it.remove();
        delete pState;
    }
}

void FlangerProcessor::initialize(EngineEffect* pEffect) {
    m_pPeriodParameter = pEffect->getParameterById("period");
    m_pDepthParameter = pEffect->getParameterById("depth");
    m_pDelayParameter = pEffect->getParameterById("delay");
}

void FlangerProcessor::process(const QString& group,
                               const CSAMPLE* pInput, CSAMPLE* pOutput,
                               const unsigned int numSamples) {
    FlangerState* pState = getStateForGroup(group);
    if (!pState) {
        qDebug() << debugString() << "WARNING: Couldn't get flanger state for group" << group;
        return;
    }

    CSAMPLE lfoPeriod = m_pPeriodParameter ? m_pPeriodParameter->value().toDouble() : 0.0f;
    CSAMPLE lfoDepth = m_pDepthParameter ? m_pDepthParameter->value().toDouble() : 0.0f;
    // Unused in EngineFlanger
    CSAMPLE lfoDelay = m_pDelayParameter ? m_pDelayParameter->value().toDouble() : 0.0f;

    qDebug() << debugString() << "period" << lfoPeriod << "depth" << lfoDepth << "delay" << lfoDelay;

    // TODO(rryan) check ranges
    // period needs to be >=0
    // delay needs to be >=0
    // depth is ???

    CSAMPLE* delayBuffer = pState->delayBuffer;
    for (int i = 0; i < numSamples; ++i) {
        delayBuffer[pState->delayPos] = pInput[i];
        pState->delayPos = (pState->delayPos + 1) % kMaxDelay;

        pState->time++;
        if (pState->time > lfoPeriod) {
            pState->time = 0;
        }

        CSAMPLE periodFraction = CSAMPLE(pState->time) / lfoPeriod;
        CSAMPLE delay = kAverageDelayLength + kLfoAmplitude * sin(two_pi * periodFraction);

        CSAMPLE prev = delayBuffer[(pState->delayPos - int(delay) + kMaxDelay - 1) % kMaxDelay];
        CSAMPLE next = delayBuffer[(pState->delayPos - int(delay) + kMaxDelay    ) % kMaxDelay];
        CSAMPLE frac = delay - floor(delay);
        CSAMPLE delayed_sample = prev + frac * (next - prev);

        pOutput[i] = pInput[i] + lfoDepth * delayed_sample;
    }
}

FlangerState* FlangerProcessor::getStateForGroup(const QString& group) {
    FlangerState* pState = m_flangerStates.value(group, NULL);
    if (pState == NULL) {
        pState = new FlangerState();
        m_flangerStates[group] = pState;
        SampleUtil::applyGain(pState->delayBuffer, 0.0f, kMaxDelay);
        pState->delayPos = 0;
        pState->time = 0;
    }
    return pState;
}
