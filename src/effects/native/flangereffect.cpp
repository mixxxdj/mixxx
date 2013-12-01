#include <QtDebug>

#include "effects/native/flangereffect.h"

#include "mathstuff.h"
#include "sampleutil.h"

QString FlangerEffect::getId() const {
    return "org.mixxx.effects.flanger";
}

EffectManifest FlangerEffect::getManifest() const {
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
    depth->setDefault(0.0f);
    depth->setMinimum(0.0f);
    depth->setMaximum(1.0f);

    EffectManifestParameter* delay = manifest.addParameter();
    delay->setId("delay");
    delay->setName(QObject::tr("Delay"));
    delay->setDescription("TODO");
    delay->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    delay->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    delay->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    delay->setDefault(50.0f);
    delay->setMinimum(50.0f);
    delay->setMaximum(10000.0f);

    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription("TODO");
    period->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    period->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    period->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    period->setDefault(50000.0f);
    period->setMinimum(50000.0f);
    period->setMaximum(2000000.0f);

    return manifest;
}

FlangerEffectProcessor::FlangerEffectProcessor(const EffectManifest& manifest) {
}

FlangerEffectProcessor::~FlangerEffectProcessor() {
    qDebug() << debugString() << "destroyed";

    QMutableMapIterator<QString, FlangerState*> it(m_flangerStates);

    while (it.hasNext()) {
        it.next();
        FlangerState* pState = it.value();
        it.remove();
        delete pState;
    }
}

void FlangerEffectProcessor::initialize(EngineEffect* pEffect) {
    m_periodParameter = pEffect->getParameterById("period");
    m_depthParameter = pEffect->getParameterById("depth");
    m_delayParameter = pEffect->getParameterById("delay");
}

void FlangerEffectProcessor::process(const QString& group,
                                     const CSAMPLE* pInput, CSAMPLE* pOutput,
                                     const unsigned int numSamples) {
    FlangerState* pState = getStateForGroup(group);

    if (!pState) {
        qDebug() << debugString() << "WARNING: Couldn't get flanger state for group" << group;
        return;
    }

    CSAMPLE lfoPeriod = m_periodParameter ? m_periodParameter->value().toDouble() : 0.0f;
    CSAMPLE lfoDepth = m_depthParameter ? m_depthParameter->value().toDouble() : 0.0f;
    // Unused in EngineFlanger
    CSAMPLE lfoDelay = m_delayParameter ? m_delayParameter->value().toDouble() : 0.0f;

    qDebug() << debugString() << "period" << lfoPeriod << "depth" << lfoDepth << "delay" << lfoDelay;

    // TODO(rryan) check ranges
    // period needs to be >=0
    // delay needs to be >=0
    // depth is ???

    for (int i = 0; i < numSamples; ++i) {
        CSAMPLE* delayBuffer = pState->delayBuffer;
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

FlangerState* FlangerEffectProcessor::getStateForGroup(const QString& group) {
    FlangerState* pState = NULL;
    if (!m_flangerStates.contains(group)) {
        pState = new FlangerState();
        m_flangerStates[group] = pState;
        SampleUtil::applyGain(pState->delayBuffer, 0.0f, kMaxDelay);
        pState->delayPos = 0;
        pState->time = 0;
    } else {
        pState = m_flangerStates[group];
    }
    return pState;
}
