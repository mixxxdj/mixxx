#include <QtDebug>

#include "effects/native/flangereffect.h"

#include "mathstuff.h"
#include "sampleutil.h"

// static
QString FlangerEffect::getId() {
    return "org.mixxx.effects.flanger";
}

// static
EffectManifestPointer FlangerEffect::getEffectManifest() {
    EffectManifest* manifest = new EffectManifest();
    manifest->setId(getId());
    manifest->setName(QObject::tr("Flanger"));
    manifest->setAuthor("The Mixxx Team");
    manifest->setVersion("1.0");
    manifest->setDescription("TODO");

    EffectManifestParameter* depth = manifest->addParameter();
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

    EffectManifestParameter* delay = manifest->addParameter();
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

    EffectManifestParameter* period = manifest->addParameter();
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

    // We don't use EffectManifestPointer here because that specifies const
    // EffectManifestParameter as the type, which does not work with
    // QObject::deleteLater.
    return QSharedPointer<EffectManifest>(manifest);
}

// static
EffectPointer FlangerEffect::create(EffectsBackend* pBackend, EffectManifestPointer pManifest) {
    return EffectPointer(new FlangerEffect(pBackend, pManifest));
}

FlangerEffect::FlangerEffect(EffectsBackend* pBackend, EffectManifestPointer pManifest)
        : Effect(pBackend, pManifest) {
    m_periodParameter = getParameterFromId("period");
    m_depthParameter = getParameterFromId("depth");
    m_delayParameter = getParameterFromId("delay");
}

FlangerEffect::~FlangerEffect() {
    qDebug() << debugString() << "destroyed";

    QMutableMapIterator<QString, FlangerState*> it(m_flangerStates);

    while (it.hasNext()) {
        it.next();
        FlangerState* pState = it.value();
        it.remove();
        delete pState;
    }
}

void FlangerEffect::process(const QString channelId,
                            const CSAMPLE* pInput, CSAMPLE* pOutput,
                            const unsigned int numSamples) {
    FlangerState* pState = getStateForChannel(channelId);

    if (!pState) {
        qDebug() << debugString() << "WARNING: Couldn't get flanger state for channel" << channelId;
        return;
    }

    CSAMPLE lfoPeriod = m_periodParameter ? m_periodParameter->getValue().toDouble() : 0.0f;
    CSAMPLE lfoDepth = m_depthParameter ? m_depthParameter->getValue().toDouble() : 0.0f;
    // Unused in EngineFlanger
    CSAMPLE lfoDelay = m_delayParameter ? m_delayParameter->getValue().toDouble() : 0.0f;

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

FlangerState* FlangerEffect::getStateForChannel(const QString channelId) {
    FlangerState* pState = NULL;
    if (!m_flangerStates.contains(channelId)) {
        pState = new FlangerState();
        m_flangerStates[channelId] = pState;
        SampleUtil::applyGain(pState->delayBuffer, 0.0f, kMaxDelay);
        pState->delayPos = 0;
        pState->time = 0;
    } else {
        pState = m_flangerStates[channelId];
    }
    return pState;
}
