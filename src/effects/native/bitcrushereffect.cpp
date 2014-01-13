#include "effects/native/bitcrushereffect.h"

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
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setValueHint(EffectManifestParameter::EffectManifestParameter::VALUE_INTEGRAL);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefault(16);
    depth->setMinimum(1);
    depth->setMaximum(16);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("downsample");
    frequency->setName(QObject::tr("Downsampling"));
    frequency->setDescription("TODO");
    frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    frequency->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    frequency->setDefault(0.0);
    frequency->setMinimum(0.0);
    frequency->setMaximum(0.9999);

    return manifest;
}

BitCrusherEffect::BitCrusherEffect(EngineEffect* pEffect,
                                   const EffectManifest& manifest)
        : m_pBitDepthParameter(pEffect->getParameterById("bit_depth")),
          m_pDownsampleParameter(pEffect->getParameterById("downsample")) {
}

BitCrusherEffect::~BitCrusherEffect() {
    qDebug() << debugString() << "destroyed";
}

void BitCrusherEffect::process(const QString& group,
                               const CSAMPLE* pInput, CSAMPLE* pOutput,
                               const unsigned int numSamples) {
    GroupState& group_state = m_groupState[group];

    // TODO(rryan) this is broken. it needs to take into account the sample
    // rate.
    const CSAMPLE downsample = m_pDownsampleParameter ?
            m_pDownsampleParameter->value().toDouble() : 0.0;
    const CSAMPLE accumulate = 1.0 - downsample;

    int bit_depth = m_pBitDepthParameter ?
            m_pBitDepthParameter->value().toInt() : 1;
    bit_depth = math_max(bit_depth, 1);

    const CSAMPLE scale = 1 << (bit_depth-1);

    const int kChannels = 2;
    for (int i = 0; i < numSamples; i += kChannels) {
        group_state.accumulator += accumulate;

        if (group_state.accumulator >= 1.0) {
            group_state.accumulator -= 1.0;
            group_state.hold_l = floorf(pInput[i] * scale + 0.5f) / scale;
            group_state.hold_r = floorf(pInput[i+1] * scale + 0.5f) / scale;
        }

        pOutput[i] = group_state.hold_l;
        pOutput[i+1] = group_state.hold_r;
    }
}
