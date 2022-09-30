#include "effects/backends/builtin/distortioneffect.h"

namespace {
inline CSAMPLE tanh_approx(CSAMPLE input) {
    return input / (1 + input * input / (3 + input * input / 5));
}
} // namespace

// static
QString DistortionEffect::getId() {
    return "org.mixxx.effects.distortion";
}

//static
EffectManifestPointer DistortionEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest);
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Distortion"));
    pManifest->setShortName(QObject::tr("Distortion"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            "A Distortion effect with several modes ranging from soft to hard "
            "clipping.");
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer mode = pManifest->addParameter();
    mode->setId("mode");
    mode->setName(QObject::tr("Mode"));
    mode->setShortName(QObject::tr("Mode"));
    mode->setDescription(QObject::tr(
            "The type of distortion to apply. Each type will sound different, "
            "ranging from soft saturation to hard clipping."));
    mode->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    mode->setRange(0, 0, 4);
    mode->appendStep(qMakePair(QObject::tr("Soft Clipping"), Mode::SoftClipping));
    mode->appendStep(qMakePair(QObject::tr("Hard Clipping"), Mode::HardClipping));
    mode->appendStep(qMakePair(QObject::tr("Soft Folding"), Mode::SoftFolding));
    mode->appendStep(qMakePair(QObject::tr("Hard Folding"), Mode::HardFolding));

    EffectManifestParameterPointer drive = pManifest->addParameter();
    drive->setId("drive");
    drive->setName(QObject::tr("Drive"));
    drive->setShortName(QObject::tr("Drive"));
    drive->setDescription(QObject::tr(
            "The amount of amplification "
            "applied to the audio signal. At higher levels the audio will be more distored."));
    drive->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    drive->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    drive->setNeutralPointOnScale(0);
    drive->setRange(0, 0, 1);

    return pManifest;
}

DistortionGroupState::DistortionGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_driveGain(1),
          m_crossfadeParameter(0),
          m_samplerate(engineParameters.sampleRate()),
          m_previousMakeUpGain(1),
          m_previousNormalizationGain(1) {
}
struct DistortionEffect::SoftClippingParameters {
    static constexpr const CSAMPLE normalizationLevel = 0.2f;
    static constexpr const CSAMPLE crossfadeEndParam = 0.2f;
    static constexpr const CSAMPLE_GAIN maxDriveGain = 25.f;

    static CSAMPLE process(CSAMPLE sample) {
        return tanh_approx(sample);
    }
};

struct DistortionEffect::HardClippingParameters {
    static constexpr const CSAMPLE normalizationLevel = 0.6f;
    static constexpr const CSAMPLE crossfadeEndParam = 0.5f;
    static constexpr const CSAMPLE_GAIN maxDriveGain = 30.f;

    static CSAMPLE process(CSAMPLE sample) {
        return CSAMPLE_clamp(sample);
    }
};

struct DistortionEffect::SoftFoldingParameters {
    static constexpr const CSAMPLE normalizationLevel = 0.3f;
    static constexpr const CSAMPLE crossfadeEndParam = 0.2f;
    static constexpr const CSAMPLE_GAIN maxDriveGain = 20.f;

    static CSAMPLE process(CSAMPLE sample) {
        return std::sin(sample);
    }
};

struct DistortionEffect::HardFoldingParameters {
    static constexpr const CSAMPLE normalizationLevel = 0.8f;
    static constexpr const CSAMPLE crossfadeEndParam = 0.2f;
    static constexpr const CSAMPLE_GAIN maxDriveGain = 15.f;

    static CSAMPLE process(CSAMPLE sample) {
        while (true) {
            if (sample > 1.0) {
                sample = 2.f - sample; // equivalent to 1 - (newSample - 1.0)
            } else if (sample < -1.0) {
                sample = -2.f - sample; // equivalent to -1 + (-newSample - 1.0)
            } else {
                return sample;
            }
        }
    }
};

void DistortionEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pMode = parameters.value("mode");
    m_pDrive = parameters.value("drive");
}

void DistortionEffect::processChannel(
        DistortionGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState);

    // TODO: anti-aliasing

    SINT numSamples = engineParameters.samplesPerBuffer();
    CSAMPLE driveParam = static_cast<CSAMPLE>(m_pDrive->value());

    if (driveParam < 0.01) {
        SampleUtil::copy(pOutput, pInput, numSamples);
        return;
    }

    switch (m_pMode->toInt()) {
    case SoftClipping:
        process<SoftClippingParameters>(driveParam, pState, pOutput, pInput, engineParameters);
        break;

    case HardClipping:
        process<HardClippingParameters>(driveParam, pState, pOutput, pInput, engineParameters);
        break;

    case SoftFolding:
        process<SoftFoldingParameters>(driveParam, pState, pOutput, pInput, engineParameters);
        break;

    case HardFolding:
        process<HardFoldingParameters>(driveParam, pState, pOutput, pInput, engineParameters);
        break;

    default:
        // We should never enter here, but we act as a noop effect just in case.
        SampleUtil::copy(pOutput, pInput, numSamples);
        return;
    }
}
