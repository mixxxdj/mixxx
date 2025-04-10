#include "effects/backends/builtin/bitcrushereffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

// static
QString BitCrusherEffect::getId() {
    return "org.mixxx.effects.bitcrusher";
}

// static
EffectManifestPointer BitCrusherEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Bitcrusher"));
    pManifest->setShortName(QObject::tr("Bitcrusher"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Adds noise by the reducing the bit depth and sample rate"));
    pManifest->setEffectRampsFromDry(true);

    EffectManifestParameterPointer depth = pManifest->addParameter();
    depth->setId("bit_depth");
    depth->setName(QObject::tr("Bit Depth"));
    depth->setShortName(QObject::tr("Bit Depth"));
    depth->setDescription(QObject::tr(
            "The bit depth of the samples"));
    depth->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    depth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    depth->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    depth->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::Inverted);
    depth->setNeutralPointOnScale(1.0);
    // for values -1 0 +1
    // we do not allow a 1 bit version because this causes a distortion because of the missing sign bit
    depth->setRange(2, 16, 16);

    EffectManifestParameterPointer frequency = pManifest->addParameter();
    frequency->setId("downsample");
    frequency->setName(QObject::tr("Downsampling"));
    frequency->setShortName(QObject::tr("Down"));
    frequency->setDescription(QObject::tr(
            "The sample rate to which the signal is downsampled"));
    frequency->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    frequency->setUnitsHint(EffectManifestParameter::UnitsHint::SampleRate);
    frequency->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    frequency->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::Inverted);
    frequency->setNeutralPointOnScale(1.0);
    frequency->setRange(0.02, 1.0, 1.0);

    return pManifest;
}

void BitCrusherEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pBitDepthParameter = parameters.value("bit_depth");
    m_pDownsampleParameter = parameters.value("downsample");
}

void BitCrusherEffect::processChannel(
        BitCrusherGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState); // no need to ramp, it is just a bitcrusher ;-)

    const auto downsample = static_cast<CSAMPLE>(
            m_pDownsampleParameter ? m_pDownsampleParameter->value() : 0.0);

    auto bit_depth = static_cast<CSAMPLE>(
            m_pBitDepthParameter ? m_pBitDepthParameter->value() : 16);

    // divided by two because we use float math which includes the sing bit anyway
    const CSAMPLE scale = std::pow(2.0f, bit_depth) / 2;
    // Gain correction is required, because MSB (values above 0.5) is usually
    // rarely used, to achieve equal loudness and maximum dynamic
    const CSAMPLE gainCorrection = (17 - bit_depth) / 8;

    for (SINT i = 0;
            i < engineParameters.samplesPerBuffer();
            i += engineParameters.channelCount()) {
        pState->accumulator += downsample;

        if (pState->accumulator >= 1.0) {
            pState->accumulator -= 1.0f;
            if (bit_depth < 16) {
                pState->hold_l = floorf(SampleUtil::clampSample(
                                                pInput[i] * gainCorrection) *
                                                 scale +
                                         0.5f) /
                        scale / gainCorrection;
                pState->hold_r = floorf(SampleUtil::clampSample(pInput[i + 1] *
                                                gainCorrection) *
                                                 scale +
                                         0.5f) /
                        scale / gainCorrection;
            } else {
                // Mixxx float has 24 bit depth, Audio CDs are 16 bit
                // here we do not change the depth
                pState->hold_l = pInput[i];
                pState->hold_r = pInput[i + 1];
            }
        }

        pOutput[i] = pState->hold_l;
        pOutput[i + 1] = pState->hold_r;
    }
}
