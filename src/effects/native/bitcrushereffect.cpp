#include "effects/native/bitcrushereffect.h"
#include "sampleutil.h"

// static
QString BitCrusherEffect::getId() {
    return "org.mixxx.effects.bitcrusher";
}

// static
EffectManifest BitCrusherEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("BitCrusher"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "The BitCrusher is an effect that adds quantisation noise to the signal "
        "by the reduction of the resolution or bandwidth of the samples."));
    manifest.setIsForFilterKnob(true);
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("bit_depth");
    depth->setName(QObject::tr("Bit Depth"));
    depth->setDescription("Adjusts the bit depth of the samples.");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    depth->setNeutralPointOnScale(1.0);
    depth->setDefault(16);
    // for values -1 0 +1
    // we do not allow a 1 bit version because this causes a distortion because of the missing sign bit
    depth->setMinimum(2);
    depth->setMaximum(16);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("downsample");
    frequency->setName(QObject::tr("Downsampling"));
    frequency->setDescription("Adjusts the sample rate, to which the signal is downsampled.");
    frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    frequency->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    frequency->setNeutralPointOnScale(1.0);
    frequency->setDefault(1.0);
    frequency->setMinimum(0.02);
    frequency->setMaximum(1.0);

    return manifest;
}

BitCrusherEffect::BitCrusherEffect(EngineEffect* pEffect,
                                   const EffectManifest& manifest)
        : m_pBitDepthParameter(pEffect->getParameterById("bit_depth")),
          m_pDownsampleParameter(pEffect->getParameterById("downsample")) {
    Q_UNUSED(manifest);
}

BitCrusherEffect::~BitCrusherEffect() {
    //qDebug() << debugString() << "destroyed";
}

void BitCrusherEffect::processGroup(const QString& group,
                                    BitCrusherGroupState* pState,
                                    const CSAMPLE* pInput, CSAMPLE* pOutput,
                                    const unsigned int numSamples,
                                    const unsigned int sampleRate,
                                    const EffectProcessor::EnableState enableState,
                                    const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate); // we are normalized to 1
    Q_UNUSED(enableState); // no need to ramp, it is just a bitcrusher ;-)

    const CSAMPLE downsample = m_pDownsampleParameter ?
            m_pDownsampleParameter->value() : 0.0;

    CSAMPLE bit_depth = m_pBitDepthParameter ?
            m_pBitDepthParameter->value() : 16;

    // divided by two because we use float math which includes the sing bit anyway
    const CSAMPLE scale = pow(2.0f, bit_depth) / 2;
    // Gain correction is required, because MSB (values above 0.5) is usually
    // rarely used, to achieve equal loudness and maximum dynamic
    const CSAMPLE gainCorrection = (17 - bit_depth) / 8;

    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        pState->accumulator += downsample;

        if (pState->accumulator >= 1.0) {
            pState->accumulator -= 1.0;
            if (bit_depth < 16) {

                pState->hold_l = floorf(SampleUtil::clampSample(pInput[i] * gainCorrection) * scale + 0.5f) / scale / gainCorrection;
                pState->hold_r = floorf(SampleUtil::clampSample(pInput[i+1] * gainCorrection) * scale + 0.5f) / scale / gainCorrection;
            } else {
                // Mixxx float has 24 bit depth, Audio CDs are 16 bit
                // here we do not change the depth
                pState->hold_l = pInput[i];
                pState->hold_r = pInput[i+1];
            }
        }

        pOutput[i] = pState->hold_l;
        pOutput[i+1] = pState->hold_r;
    }
}
