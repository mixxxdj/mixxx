#include "effects/builtin/flangereffect.h"

#include <QtDebug>

#include "util/math.h"

namespace{

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
    pManifest->setDescription(QObject::tr(
        "Mixes the input with a delayed, pitch modulated copy of itself to create comb filtering"));
    pManifest->setMetaknobDefault(1.0);

    EffectManifestParameterPointer speed = pManifest->addParameter();
    speed->setId("speed");
    speed->setName(QObject::tr("Speed"));
    speed->setShortName(QObject::tr("Speed"));
    speed->setDescription(QObject::tr(
        "Speed of the LFO (low frequency oscillator)\n"
        "32 - 1/4 beats rounded to 1/2 beat per LFO cycle if tempo is detected\n"
        "1/32 - 4 Hz if no tempo is detected"));
    speed->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC_INVERSE);
    speed->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    speed->setMinimum(kMinLfoBeats);
    speed->setMaximum(kMaxLfoBeats);
    speed->setDefault(8);

    EffectManifestParameterPointer width = pManifest->addParameter();
    width->setId("width");
    width->setName(QObject::tr("Width"));
    width->setShortName(QObject::tr("Width"));
    width->setDescription(QObject::tr(
        "Delay amplitude of the LFO (low frequency oscillator)"));
    width->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    width->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    width->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    width->setDefault(kMaxLfoWidthMs / 2);
    width->setMinimum(0.0);
    width->setMaximum(kMaxLfoWidthMs);

    EffectManifestParameterPointer manual = pManifest->addParameter();
    manual->setId("manual");
    manual->setName(QObject::tr("Manual"));
    manual->setShortName(QObject::tr("Manual"));
    manual->setDescription(QObject::tr(
        "Delay offset of the LFO (low frequency oscillator).\n"
        "With width at zero, this allows for manually sweeping over the entire delay range."));
    manual->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    manual->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    manual->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    manual->setDefault(kCenterDelayMs);
    manual->setMinimum(kMinDelayMs);
    manual->setMaximum(kMaxDelayMs);

    EffectManifestParameterPointer regen = pManifest->addParameter();
    regen->setId("regen");
    regen->setName(QObject::tr("Regeneration"));
    regen->setShortName(QObject::tr("Regen"));
    regen->setDescription(QObject::tr(
           "How much of the delay output is feed back into the input"));
    regen->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    regen->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    regen->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    regen->setDefault(0.25);
    regen->setMinimum(0.0);
    regen->setMaximum(1.0);

    EffectManifestParameterPointer mix = pManifest->addParameter();
    mix->setId("mix");
    mix->setName(QObject::tr("Mix"));
    mix->setShortName(QObject::tr("Mix"));
    mix->setDescription(QObject::tr(
            "Intensity of the effect"));
    mix->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    mix->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    mix->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    mix->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    mix->setDefault(1.0);
    mix->setMinimum(0.0);
    mix->setMaximum(1.0);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(QObject::tr(
            "Divide rounded 1/2 beats of the Period parameter by 3."));
    triplet->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    return pManifest;
}

FlangerEffect::FlangerEffect(EngineEffect* pEffect)
        : m_pSpeedParameter(pEffect->getParameterById("speed")),
          m_pWidthParameter(pEffect->getParameterById("width")),
          m_pManualParameter(pEffect->getParameterById("manual")),
          m_pRegenParameter(pEffect->getParameterById("regen")),
          m_pMixParameter(pEffect->getParameterById("mix")),
          m_pTripletParameter(pEffect->getParameterById("triplet")) {
}

FlangerEffect::~FlangerEffect() {
    //qDebug() << debugString() << "destroyed";
}

void FlangerEffect::processChannel(const ChannelHandle& handle,
                                   FlangerGroupState* pState,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const mixxx::EngineParameters& bufferParameters,
                                   const EffectEnableState enableState,
                                   const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);

    double lfoPeriodParameter = m_pSpeedParameter->value();
    double lfoPeriodFrames;
    if (groupFeatures.has_beat_length_sec) {
        // lfoPeriodParameter is a number of beats
        lfoPeriodParameter = std::max(roundToFraction(lfoPeriodParameter, 2.0), kMinLfoBeats);
        if (m_pTripletParameter->toBool()) {
            lfoPeriodParameter /= 3.0;
        }
        lfoPeriodFrames = lfoPeriodParameter * groupFeatures.beat_length_sec
                * bufferParameters.sampleRate();
    } else {
        // lfoPeriodParameter is a number of seconds
        lfoPeriodFrames = std::max(lfoPeriodParameter, kMinLfoBeats)
                * bufferParameters.sampleRate();
    }

    // When the period is changed, the position of the sound shouldn't
    // so time need to be recalculated
    if (pState->previousPeriodFrames != -1.0) {
        pState->lfoFrames *= static_cast<unsigned int>(
                lfoPeriodFrames / pState->previousPeriodFrames);
    }
    pState->previousPeriodFrames = lfoPeriodFrames;


    // lfoPeriodSamples is used to calculate the delay for each channel
    // independently in the loop below, so do not multiply lfoPeriodSamples by
    // the number of channels.

    const auto mix = static_cast<CSAMPLE_GAIN>(m_pMixParameter->value());
    RampingValue<CSAMPLE_GAIN> mixRamped(
            pState->prev_mix, mix, bufferParameters.framesPerBuffer());
    pState->prev_mix = mix;

    const auto regen = static_cast<CSAMPLE_GAIN>(m_pRegenParameter->value());
    RampingValue<CSAMPLE_GAIN> regenRamped(
            pState->prev_regen, regen, bufferParameters.framesPerBuffer());
    pState->prev_regen = regen;

    // With and Manual is limited by amount of amplitude that remains from width
    // to kMaxDelayMs
    double width = m_pWidthParameter->value();
    double manual = m_pManualParameter->value();
    double maxManual = kCenterDelayMs + (kMaxLfoWidthMs - width) / 2;
    double minManual = kCenterDelayMs - (kMaxLfoWidthMs - width) / 2;
    manual = math_clamp(manual, minManual, maxManual);

    RampingValue<double> widthRamped(
            pState->prev_width, width, bufferParameters.framesPerBuffer());
    pState->prev_width = static_cast<CSAMPLE_GAIN>(width);

    RampingValue<double> manualRamped(
            pState->prev_manual, manual, bufferParameters.framesPerBuffer());
    pState->prev_manual = static_cast<CSAMPLE_GAIN>(manual);

    CSAMPLE* delayLeft = pState->delayLeft;
    CSAMPLE* delayRight = pState->delayRight;

    for (SINT i = 0;
            i < bufferParameters.samplesPerBuffer();
            i += bufferParameters.channelCount()) {
        CSAMPLE_GAIN mix_ramped = mixRamped.getNext();
        CSAMPLE_GAIN regen_ramped = regenRamped.getNext();
        double width_ramped = widthRamped.getNext();
        double manual_ramped = manualRamped.getNext();

        pState->lfoFrames++;
        if (pState->lfoFrames >= lfoPeriodFrames) {
            pState->lfoFrames = 0;
        }

        auto periodFraction = pState->lfoFrames / static_cast<float>(lfoPeriodFrames);
        double delayMs = manual_ramped + width_ramped / 2 * sin(M_PI * 2.0f * periodFraction);
        double delayFrames = delayMs * bufferParameters.sampleRate() / 1000;

        SINT framePrev = (pState->delayPos - static_cast<SINT>(floor(delayFrames))
                + kBufferLenth) % kBufferLenth;
        SINT frameNext = (pState->delayPos - static_cast<SINT>(ceil(delayFrames))
                + kBufferLenth) % kBufferLenth;
        CSAMPLE prevLeft = delayLeft[framePrev];
        CSAMPLE nextLeft = delayLeft[frameNext];

        CSAMPLE prevRight = delayRight[framePrev];
        CSAMPLE nextRight = delayRight[frameNext];

        const CSAMPLE_GAIN frac = static_cast<CSAMPLE_GAIN>(
                delayFrames - floorf(static_cast<float>(delayFrames)));
        CSAMPLE delayedSampleLeft = prevLeft + frac * (nextLeft - prevLeft);
        CSAMPLE delayedSampleRight = prevRight + frac * (nextRight - prevRight);

        delayLeft[pState->delayPos] = tanh_approx(pInput[i] + regen_ramped * delayedSampleLeft);
        delayRight[pState->delayPos] = tanh_approx(pInput[i + 1] + regen_ramped * delayedSampleRight);

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
