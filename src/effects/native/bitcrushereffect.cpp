#include "effects/native/bitcrushereffect.h"

QString BitCrusherEffect::getId() const {
    return "org.mixxx.effects.bitcrusher";
}

EffectManifest BitCrusherEffect::getManifest() const {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("BitCrusher"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("TODO");

    EffectManifestParameter* depth = manifest.addParameter();
    depth->setId("depth");
    depth->setName(QObject::tr("Depth"));
    depth->setDescription("TODO");
    depth->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    depth->setValueHint(EffectManifestParameter::EffectManifestParameter::VALUE_INTEGRAL);
    depth->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    depth->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    depth->setDefault(0);
    depth->setMinimum(0);
    depth->setMaximum(16);

    EffectManifestParameter* frequency = manifest.addParameter();
    frequency->setId("frequency");
    frequency->setName(QObject::tr("Frequency"));
    frequency->setDescription("TODO");
    frequency->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    frequency->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    frequency->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    frequency->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    frequency->setDefault(1.0);
    frequency->setMinimum(0.0);
    frequency->setMaximum(1.0);

    return manifest;
}

BitCrusherProcessor::BitCrusherProcessor(const EffectManifest& manifest)
        : m_pDepthParameter(NULL),
          m_pFrequencyParameter(NULL) {
}

BitCrusherProcessor::~BitCrusherProcessor() {
    qDebug() << debugString() << "destroyed";
}

void BitCrusherProcessor::initialize(EngineEffect* pEffect) {
    m_pDepthParameter = pEffect->getParameterById("depth");
    m_pFrequencyParameter = pEffect->getParameterById("frequency");
}

void BitCrusherProcessor::process(const QString& group,
                                  const CSAMPLE* pInput, CSAMPLE* pOutput,
                                  const unsigned int numSamples) {
    ChannelState& group_state = m_groupState[group];

    CSAMPLE frequency = m_pFrequencyParameter ?
            m_pFrequencyParameter->value().toDouble() : 1.0;

    int depth = m_pDepthParameter ? m_pDepthParameter->value().toInt() : 0;
    const CSAMPLE step = 1.0 / pow(2.0, static_cast<double>(depth));

    const int kChannels = 2;
    const int numFrames = numSamples / kChannels;

    for (int i = 0; i < numFrames; i += kChannels) {
        group_state.phasor += frequency;

        if (group_state.phasor >= 1.0) {
            group_state.phasor -= 1.0;

            pOutput[i] = step * floorf(pInput[i]/step + 0.5);
            pOutput[i+1] = step * floorf(pInput[i+1]/step + 0.5);
        }
    }
}
