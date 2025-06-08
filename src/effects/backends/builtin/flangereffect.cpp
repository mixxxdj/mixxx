#include "effects/backends/builtin/flangereffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"

namespace {

// Gain correction was verified with replay gain and default parameters
constexpr CSAMPLE kGainCorrection = 1.41253754f; // 3 dB

inline CSAMPLE tanh_approx(CSAMPLE input) {
    // return tanhf(input); // 142ns for process;
    return input / (1 + input * input / (3 + input * input / 5)); // 119ns for process
}
} // namespace

// static
QString FlangerEffect::getId() {
    return "org.mixxx.effects.flanger";
}

// static
EffectManifestPointer FlangerEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Flanger"));
    pManifest->setShortName(QObject::tr("Flanger"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("Mixes the input with a delayed, pitch modulated copy "
                        "of itself to create comb filtering"));

    EffectManifestParameterPointer speed = pManifest->addParameter();
    speed->setId("speed");
    speed->setName(QObject::tr("Speed"));
    speed->setShortName(QObject::tr("Speed"));
    speed->setDescription(QObject::tr(
            "Speed of the LFO (low frequency oscillator)\n"
            "32 - 1/4 beats rounded to 1/2 beat per LFO cycle if tempo is detected\n"
            "1/32 - 4 Hz if no tempo is detected"));
    speed->setValueScaler(EffectManifestParameter::ValueScaler::LogarithmicInverse);
    speed->setRange(kMinLfoBeats, 8, kMaxLfoBeats);

    EffectManifestParameterPointer width = pManifest->addParameter();
    width->setId("width");
    width->setName(QObject::tr("Width"));
    width->setShortName(QObject::tr("Width"));
    width->setDescription(QObject::tr(
            "Delay amplitude of the LFO (low frequency oscillator)"));
    width->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    width->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    width->setRange(0.0, kMaxLfoWidthMs / 2, kMaxLfoWidthMs);

    EffectManifestParameterPointer manual = pManifest->addParameter();
    manual->setId("manual");
    manual->setName(QObject::tr("Manual"));
    manual->setShortName(QObject::tr("Manual"));
    manual->setDescription(QObject::tr(
            "Delay offset of the LFO (low frequency oscillator).\n"
            "With width at zero, this allows for manually sweeping over the entire delay range."));
    manual->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    manual->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    manual->setRange(kMinDelayMs, kCenterDelayMs, kMaxDelayMs);

    EffectManifestParameterPointer regen = pManifest->addParameter();
    regen->setId("regen");
    regen->setName(QObject::tr("Regeneration"));
    regen->setShortName(QObject::tr("Regen"));
    regen->setDescription(QObject::tr(
            "How much of the delay output is feed back into the input"));
    regen->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    regen->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    regen->setRange(0.0, 0.25, 1.0);

    EffectManifestParameterPointer mix = pManifest->addParameter();
    mix->setId("mix");
    mix->setName(QObject::tr("Mix"));
    mix->setShortName(QObject::tr("Mix"));
    mix->setDescription(QObject::tr(
            "Intensity of the effect"));
    mix->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    mix->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    mix->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    mix->setRange(0.0, 1.0, 1.0);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(QObject::tr(
            "Divide rounded 1/2 beats of the Period parameter by 3."));
    triplet->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    triplet->setRange(0, 0, 1);

    return pManifest;
}

void FlangerEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pSpeedParameter = parameters.value("speed");
    m_pWidthParameter = parameters.value("width");
    m_pManualParameter = parameters.value("manual");
    m_pRegenParameter = parameters.value("regen");
    m_pMixParameter = parameters.value("mix");
    m_pTripletParameter = parameters.value("triplet");
}

void FlangerEffect::processChannel(
        FlangerGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    double lfoPeriodParameter = m_pSpeedParameter->value();
    double lfoPeriodFrames;
    if (groupFeatures.beat_length.has_value()) {
        // lfoPeriodParameter is a number of beats
        lfoPeriodParameter = std::max(roundToFraction(lfoPeriodParameter, 2.0), kMinLfoBeats);
        if (m_pTripletParameter->toBool()) {
            lfoPeriodParameter /= 3.0;
        }
        lfoPeriodFrames = lfoPeriodParameter * groupFeatures.beat_length->frames;
    } else {
        // lfoPeriodParameter is a number of seconds
        lfoPeriodFrames = std::max(lfoPeriodParameter, kMinLfoBeats) *
                engineParameters.sampleRate();
    }

    // When the period is changed, the position of the sound shouldn't
    // so time need to be recalculated
    if (pState->previousPeriodFrames != -1.0) {
        pState->lfoFrames = static_cast<unsigned int>(
                lfoPeriodFrames / pState->previousPeriodFrames * pState->lfoFrames);
    }
    pState->previousPeriodFrames = lfoPeriodFrames;

    // lfoPeriodSamples is used to calculate the delay for each channel
    // independently in the loop below, so do not multiply lfoPeriodSamples by
    // the number of channels.

    const auto mix = static_cast<CSAMPLE_GAIN>(m_pMixParameter->value());
    RampingValue<CSAMPLE_GAIN> mixRamped(
            pState->prev_mix, mix, engineParameters.framesPerBuffer());
    pState->prev_mix = mix;

    const auto regen = static_cast<CSAMPLE_GAIN>(m_pRegenParameter->value());
    RampingValue<CSAMPLE_GAIN> regenRamped(
            pState->prev_regen, regen, engineParameters.framesPerBuffer());
    pState->prev_regen = regen;

    // With and Manual is limited by amount of amplitude that remains from width
    // to kMaxDelayMs
    double width = m_pWidthParameter->value();
    double manual = m_pManualParameter->value();
    double maxManual = kCenterDelayMs + (kMaxLfoWidthMs - width) / 2;
    double minManual = kCenterDelayMs - (kMaxLfoWidthMs - width) / 2;
    manual = math_clamp(manual, minManual, maxManual);

    RampingValue<double> widthRamped(
            pState->prev_width, width, engineParameters.framesPerBuffer());
    pState->prev_width = static_cast<CSAMPLE_GAIN>(width);

    RampingValue<double> manualRamped(
            pState->prev_manual, manual, engineParameters.framesPerBuffer());
    pState->prev_manual = static_cast<CSAMPLE_GAIN>(manual);

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

    int rampIndex = 0;
    for (SINT i = 0;
            i < engineParameters.samplesPerBuffer();
            i += engineParameters.channelCount()) {
        CSAMPLE_GAIN mix_ramped = mixRamped.getNth(rampIndex);
        CSAMPLE_GAIN regen_ramped = regenRamped.getNth(rampIndex);
        double width_ramped = widthRamped.getNth(rampIndex);
        double manual_ramped = manualRamped.getNth(rampIndex);
        ++rampIndex;

        pState->lfoFrames++;
        if (pState->lfoFrames >= lfoPeriodFrames) {
            pState->lfoFrames = 0;
        }

        auto periodFraction = pState->lfoFrames / static_cast<float>(lfoPeriodFrames);
        double delayMs = manual_ramped + width_ramped / 2 * sin(M_PI * 2.0f * periodFraction);
        double delayFrames = delayMs * engineParameters.sampleRate() / 1000;

        SINT framePrev =
                (pState->delayPos - static_cast<SINT>(floor(delayFrames)) +
                        kBufferLenth) %
                kBufferLenth;
        SINT frameNext =
                (pState->delayPos - static_cast<SINT>(ceil(delayFrames)) +
                        kBufferLenth) %
                kBufferLenth;
        CSAMPLE prevLeft = delayLeft[framePrev];
        CSAMPLE nextLeft = delayLeft[frameNext];

        CSAMPLE prevRight = delayRight[framePrev];
        CSAMPLE nextRight = delayRight[frameNext];

        const CSAMPLE_GAIN frac = static_cast<CSAMPLE_GAIN>(
                delayFrames - floorf(static_cast<float>(delayFrames)));
        CSAMPLE delayedSampleLeft = prevLeft + frac * (nextLeft - prevLeft);
        CSAMPLE delayedSampleRight = prevRight + frac * (nextRight - prevRight);

        delayLeft[pState->delayPos] = tanh_approx(pInput[i] + regen_ramped * delayedSampleLeft);
        delayRight[pState->delayPos] =
                tanh_approx(pInput[i + 1] + regen_ramped * delayedSampleRight);

        pState->delayPos = (pState->delayPos + 1) % kBufferLenth;

        CSAMPLE_GAIN gain = (1 - mix_ramped + kGainCorrection * mix_ramped);
        pOutput[i] = (pInput[i] + mix_ramped * delayedSampleLeft) / gain;
        pOutput[i + 1] = (pInput[i + 1] + mix_ramped * delayedSampleRight) / gain;
    }

    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::clear(delayLeft, kBufferLenth);
        SampleUtil::clear(delayRight, kBufferLenth);
        pState->previousPeriodFrames = -1;
        pState->prev_regen = 0;
        pState->prev_mix = 0;
    }
}
