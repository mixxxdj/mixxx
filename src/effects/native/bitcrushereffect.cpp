#include "effects/native/bitcrushereffect.h"

#include "util/sample.h"

// static
QString BitCrusherEffect::getId() {
    return "org.mixxx.effects.bitcrusher";
}

// static
EffectManifest BitCrusherEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Bitcrusher"));
    manifest.setShortName(QObject::tr("Bitcrusher"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "Adds noise by the reducing the bit depth and sample rate"));
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("bit_depth");
    depth->setName(QObject::tr("Bit Depth"));
    depth->setShortName(QObject::tr("Bit Depth"));
    depth->setDescription(QObject::tr(
        "The bit depth of the samples"));
    depth->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    depth->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    depth->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    depth->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::INVERTED);
    depth->setNeutralPointOnScale(1.0);
    depth->setDefault(16);
    // for values -1 0 +1
    // we do not allow a 1 bit version because this causes a distortion because of the missing sign bit
    depth->setMinimum(2);
    depth->setMaximum(16);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("downsample");
    frequency->setName(QObject::tr("Downsampling"));
    frequency->setShortName(QObject::tr("Down"));
    frequency->setDescription(QObject::tr(
        "The sample rate to which the signal is downsampled"));
    frequency->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    frequency->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UnitsHint::SAMPLERATE);
    frequency->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    frequency->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::INVERTED);
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

void BitCrusherEffect::processChannel(const ChannelHandle& handle,
                                      BitCrusherGroupState* pState,
                                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                                      const mixxx::EngineParameters& bufferParameters,
                                      const EffectEnableState enableState,
                                      const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);
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

    // The character of this effect changes with the input level and effects are processed
    // after the volume fader. So, to preserve the character of the effect as the volume fader
    // is lowered, undo the attenuation from the volume fader on the input for this effect, then
    // reapply it on the output.
    const double faderGainDelta = (groupFeatures.volume_fader_new - groupFeatures.volume_fader_old) /
            bufferParameters.framesPerBuffer();
    const double faderStartGain = groupFeatures.volume_fader_old + faderGainDelta;


    for (unsigned int i = 0;
            i < bufferParameters.samplesPerBuffer();
            i += bufferParameters.channelCount()) {
        pState->accumulator += downsample;

        // Note that this is an attenuation on a 0 - 1 scale, so dividing samples by it undoes
        // the attenuation.
        const CSAMPLE_GAIN faderGain = faderStartGain + faderGainDelta
                * (i / bufferParameters.channelCount());
        DEBUG_ASSERT(faderGain >= 0.0 && faderGain <= 1.0);

        if (pState->accumulator >= 1.0) {
            pState->accumulator -= 1.0;
            if (bit_depth < 16) {
                CSAMPLE clamped = SampleUtil::clampSample(pInput[i] * gainCorrection / faderGain);
                pState->hold_l = floorf(clamped * scale + 0.5f) / scale / gainCorrection * faderGain;

                clamped = SampleUtil::clampSample(pInput[i+1] * gainCorrection / faderGain);
                pState->hold_r = floorf(clamped * scale + 0.5f) / scale / gainCorrection * faderGain;
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
