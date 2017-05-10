#include "effects/native/flangereffect.h"

#include <QtDebug>

#include "util/math.h"

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

    EffectManifestParameter* period = manifest.addParameter();
    period->setId("period");
    period->setName(QObject::tr("Period"));
    period->setDescription(QObject::tr("Controls the period of the LFO (low frequency oscillator)\n"
        "0 - 4 beats if sync parameter is enabled and tempo is detected (decks and samplers) \n"
        "0 - 4 seconds if sync parameter is disabled or no tempo is detected (mic & aux inputs, master mix)"));
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    period->setMinimum(0.00);
    period->setMaximum(4.00);
    period->setDefault(0.50);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription(QObject::tr("Controls the intensity of the effect."));
    depth->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    depth->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    depth->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    depth->setDefault(1.0);
    depth->setMinimum(0.0);
    depth->setMaximum(1.0);

    EffectManifestParameter* sync = manifest.addParameter();
    sync->setId("sync");
    sync->setName(QObject::tr("Sync"));
    sync->setDescription(QObject::tr("Synchronizes the period with the tempo if it can be retrieved"));
    sync->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    sync->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    sync->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    sync->setDefault(1);
    sync->setMinimum(0);
    sync->setMaximum(1);

    return manifest;
}

FlangerEffect::FlangerEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pSyncParameter(pEffect->getParameterById("sync")) {
    Q_UNUSED(manifest);
}

FlangerEffect::~FlangerEffect() {
    //qDebug() << debugString() << "destroyed";
}

void FlangerEffect::processChannel(const ChannelHandle& handle,
                                   FlangerGroupState* pState,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const unsigned int numSamples,
                                   const unsigned int sampleRate,
                                   const EffectProcessor::EnableState enableState,
                                   const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    // TODO: remove assumption of stereo signal
    const int kChannels = 2;

    // The parameter minimum is zero so the exact center of the knob is 2 beats.
    CSAMPLE periodTime = std::max(m_pPeriodParameter->value(), 0.05);
    CSAMPLE lfoPeriod;
    if (m_pSyncParameter->toBool() && groupFeatures.has_beat_length) {
        // period is a number of beats
        lfoPeriod = periodTime * groupFeatures.beat_length;
    } else {
        // period is a number of seconds
        lfoPeriod = periodTime * sampleRate * kChannels;
    }

    CSAMPLE lfoDepth = m_pDepthParameter->value();

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

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
