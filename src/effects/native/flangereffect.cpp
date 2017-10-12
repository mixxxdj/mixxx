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
        "1/4 - 4 beats rounded to 1/2 beat if tempo is detected (decks and samplers) \n"
        "0.05 - 4 seconds if no tempo is detected (mic & aux inputs, master mix)"));
    period->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    period->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    period->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    period->setMinimum(0.00);
    period->setMaximum(4.00);
    period->setDefault(1.00);

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

    EffectManifestParameter* triplet = manifest.addParameter();
    triplet->setId("triplet");
    triplet->setName("Triplets");
    triplet->setDescription("Divide rounded 1/2 beats of the Period parameter by 3.");
    triplet->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    return manifest;
}

FlangerEffect::FlangerEffect(EngineEffect* pEffect,
                             const EffectManifest& manifest)
        : m_pPeriodParameter(pEffect->getParameterById("period")),
          m_pDepthParameter(pEffect->getParameterById("depth")),
          m_pTripletParameter(pEffect->getParameterById("triplet")) {
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

    // TODO: remove assumption of stereo signal
    const int kChannels = 2;

    // The parameter minimum is zero so the exact center of the knob is 2 beats.
    double lfoPeriodParameter = m_pPeriodParameter->value();
    double lfoPeriodSamples;
    if (groupFeatures.has_beat_length_sec) {
        // lfoPeriodParameter is a number of beats
        lfoPeriodParameter = std::max(roundToFraction(lfoPeriodParameter, 2.0), 1/4.0);
        if (m_pTripletParameter->toBool()) {
            lfoPeriodParameter /= 3.0;
        }
        lfoPeriodSamples = lfoPeriodParameter * groupFeatures.beat_length_sec * sampleRate;
    } else {
        // lfoPeriodParameter is a number of seconds
        lfoPeriodSamples = std::max(lfoPeriodParameter, 1/4.0) * sampleRate;
    }
    // lfoPeriodSamples is used to calculate the delay for each channel
    // independently in the loop below, so do not multiply lfoPeriodSamples by
    // the number of channels.

    CSAMPLE lfoDepth = m_pDepthParameter->value();

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        delayLeft[pState->delayPos] = pInput[i];
        delayRight[pState->delayPos] = pInput[i+1];

        pState->delayPos = (pState->delayPos + 1) % kMaxDelay;

        pState->time++;
        if (pState->time > lfoPeriodSamples) {
            pState->time = 0;
        }

        CSAMPLE periodFraction = CSAMPLE(pState->time) / lfoPeriodSamples;
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

    if (enableState == EffectProcessor::DISABLING) {
        SampleUtil::clear(delayLeft, numSamples);
        SampleUtil::clear(delayRight, numSamples);
    }
}
