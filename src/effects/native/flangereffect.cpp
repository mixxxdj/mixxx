#include "effects/native/flangereffect.h"

#include <QtDebug>

#include "util/math.h"

namespace{
inline CSAMPLE tanh_approx(CSAMPLE input) {
    // return tanhf(input); // 142ns for process;
    return input / (1 + input * input / (3 + input * input / 5)); // 119ns for process
}
}

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

    EffectManifestParameter* speed = manifest.addParameter();
    speed->setId("speed");
    speed->setName(QObject::tr("Speed"));
    speed->setDescription(QObject::tr("Controls the speed of the LFO (low frequency oscillator)\n"
        "1/4 - 4 beats rounded to 1/2 beat if tempo is detected (decks and samplers) \n"
        "0.05 - 4 seconds if no tempo is detected (mic & aux inputs, master mix)"));
    speed->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    speed->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    speed->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    speed->setMinimum(0.00);
    speed->setMaximum(4.00);
    speed->setDefault(1.00);

    EffectManifestParameter* width = manifest.addParameter();
    width->setId("width");
    width->setName(QObject::tr("Width"));
    width->setDescription(QObject::tr("Controls the delay amplitude of the LFO (low frequency oscillator)."));
    width->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    width->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    width->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    width->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    width->setDefault(kMaxLfoWidthMs);
    width->setMinimum(0.0);
    width->setMaximum(kMaxLfoWidthMs);

    EffectManifestParameter* manual = manifest.addParameter();
    manual->setId("manual");
    manual->setName(QObject::tr("Manual"));
    manual->setDescription(QObject::tr("Controls the delay offset of the LFO (low frequency oscillator).\n"
            "With width at zero, it allows to manual sweep over the entire delay range."));
    manual->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    manual->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    manual->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    manual->setDefault(kCenterDelayMs);
    manual->setMinimum(kMinDelayMs);
    manual->setMaximum(kMaxDelayMs);

    EffectManifestParameter* regen = manifest.addParameter();
    regen->setId("regen");
    regen->setName(QObject::tr("Regen."));
    regen->setDescription(QObject::tr("Controls how much of the delay output is feed back into the input."));
    regen->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    regen->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    regen->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    regen->setDefault(0.0);
    regen->setMinimum(0.0);
    regen->setMaximum(1.0);

    EffectManifestParameter* mix = manifest.addParameter();
    mix->setId("mix");
    mix->setName(QObject::tr("Mix"));
    mix->setDescription(QObject::tr("Controls the intensity of the effect."));
    mix->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    mix->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    mix->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    mix->setDefault(1.0);
    mix->setMinimum(0.0);
    mix->setMaximum(1.0);

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
        : m_pSpeedParameter(pEffect->getParameterById("speed")),
          m_pWidthParameter(pEffect->getParameterById("width")),
          m_pManualParameter(pEffect->getParameterById("manual")),
          m_pRegenParameter(pEffect->getParameterById("regen")),
          m_pMixParameter(pEffect->getParameterById("mix")),
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
    double lfoSpeedParameter = m_pSpeedParameter->value();
    double lfoPeriodFrames;
    if (groupFeatures.has_beat_length_sec) {
        // lfoPeriodParameter is a number of beats
        lfoSpeedParameter = std::max(roundToFraction(lfoSpeedParameter, 2.0), 1/4.0);
        if (m_pTripletParameter->toBool()) {
            lfoSpeedParameter /= 3.0;
        }
        lfoPeriodFrames = lfoSpeedParameter * groupFeatures.beat_length_sec * sampleRate;
    } else {
        // lfoPeriodParameter is a number of seconds
        lfoPeriodFrames = std::max(lfoSpeedParameter, 1/4.0) * sampleRate;
    }
    // lfoPeriodSamples is used to calculate the delay for each channel
    // independently in the loop below, so do not multiply lfoPeriodSamples by
    // the number of channels.

    CSAMPLE_GAIN mix = m_pMixParameter->value();
    CSAMPLE_GAIN regen = m_pRegenParameter->value();

    // With and Manual is limited by amount of amplitude that remains from width
    // to kMaxDelaMS
    double width = m_pWidthParameter->value();
    double manual = m_pManualParameter->value();
    double maxManual = kCenterDelayMs + (kMaxLfoWidthMs - width) / 2;
    double minManual = kCenterDelayMs - (kMaxLfoWidthMs - width) / 2;
    manual = math_clamp(manual, minManual, maxManual);

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

    for (unsigned int i = 0; i < numSamples; i += kChannels) {

        pState->lfoFrames++;
        if (pState->lfoFrames >= lfoPeriodFrames) {
            pState->lfoFrames = 0;
        }

        float periodFraction = static_cast<float>(pState->lfoFrames) / lfoPeriodFrames;
        double delayMs = manual + width / 2 * sin(M_PI * 2.0f * periodFraction);
        double delayFrames = delayMs * sampleRate / 1000;

        SINT framePrev = (pState->delayPos - static_cast<SINT>(floor(delayFrames))
                + kBufferLenth) % kBufferLenth;
        SINT frameNext = (pState->delayPos - static_cast<SINT>(ceil(delayFrames))
                + kBufferLenth) % kBufferLenth;
        CSAMPLE prevLeft = delayLeft[framePrev];
        CSAMPLE nextLeft = delayLeft[frameNext];

        CSAMPLE prevRight = delayRight[framePrev];
        CSAMPLE nextRight = delayRight[frameNext];

        CSAMPLE frac = delayFrames - floorf(delayFrames);
        CSAMPLE delayedSampleLeft = prevLeft + frac * (nextLeft - prevLeft);
        CSAMPLE delayedSampleRight = prevRight + frac * (nextRight - prevRight);

        delayLeft[pState->delayPos] = tanh_approx(pInput[i] + regen * delayedSampleLeft);
        delayRight[pState->delayPos] = tanh_approx(pInput[i + 1] + regen * delayedSampleRight);

        pState->delayPos = (pState->delayPos + 1) % kBufferLenth;

        pOutput[i] = pInput[i] + mix * delayedSampleLeft;
        pOutput[i+1] = pInput[i + 1] + mix * delayedSampleRight;
    }

    if (enableState == EffectProcessor::DISABLING) {
        SampleUtil::clear(delayLeft, numSamples);
        SampleUtil::clear(delayRight, numSamples);
    }
}
