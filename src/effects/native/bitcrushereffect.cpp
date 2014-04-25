#include "effects/native/bitcrushereffect.h"
#include "util/math.h"

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
    manifest.setDescription("TODO");

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("bit_depth");
    depth->setName(QObject::tr("Bit Depth"));
    depth->setDescription("TODO");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    depth->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setLinkHint(EffectManifestParameter::LINK_INVERSE);
    depth->setDefault(16);
    depth->setMinimum(1);
    depth->setMaximum(16);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("downsample");
    frequency->setName(QObject::tr("Downsampling"));
    frequency->setDescription("TODO");
    frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    frequency->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    frequency->setLinkHint(EffectManifestParameter::LINK_INVERSE);
    frequency->setDefault(1.0);
    frequency->setMinimum(0.0);
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
                                    const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    // TODO(rryan) this is broken. it needs to take into account the sample
    // rate.
    const CSAMPLE downsample = m_pDownsampleParameter ?
            m_pDownsampleParameter->value().toDouble() : 0.0;

    CSAMPLE bit_depth = m_pBitDepthParameter ?
            m_pBitDepthParameter->value().toDouble() : 1.0;
    bit_depth = math_max(bit_depth, 1.0f);

    const CSAMPLE scale = pow(2.0f, bit_depth - 1);

    const int kChannels = 2;
    for (unsigned int i = 0; i < numSamples; i += kChannels) {
        pState->accumulator += downsample;

        if (pState->accumulator >= 1.0) {
            pState->accumulator -= 1.0;
            pState->hold_l = floorf(pInput[i] * scale + 0.5f) / scale;
            pState->hold_r = floorf(pInput[i+1] * scale + 0.5f) / scale;
        }

        pOutput[i] = pState->hold_l;
        pOutput[i+1] = pState->hold_r;
    }
}
