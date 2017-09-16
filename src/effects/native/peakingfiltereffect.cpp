#include "effects/native/peakingfiltereffect.h"
#include "util/math.h"


// static
QString PeakingFilterEffect::getId() {
    return "org.mixxx.effects.peakingfilter";
}

// static
EffectManifest PeakingFilterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Peaking Filter"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A peaking or notch filter based on a Biquad Filter"));

    EffectManifestParameter* q = manifest.addParameter();
    q->setId(QString("q"));
    q->setName(QString("Q"));
    q->setDescription(QString("Q"));
    q->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    q->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    q->setDefault(1);
    q->setMinimum(0.1);
    q->setMaximum(10);

    EffectManifestParameter* gain = manifest.addParameter();
    gain->setId(QString("gain"));
    gain->setName(QString("Gain"));
    gain->setDescription(QString("Gain"));
    gain->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    gain->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    gain->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    gain->setDefault(0);
    gain->setMinimum(-48);
    gain->setMaximum(+48);

    EffectManifestParameter* center = manifest.addParameter();
    center->setId(QString("center"));
    center->setName(QString("Center"));
    center->setDescription(QString("Center frequency"));
    center->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    center->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    center->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    center->setDefault(0.022675737); // 1000 Hz @ 44100
    center->setMinimum(0.003);
    center->setMaximum(0.5);

    return manifest;
}

PeakingFilterEffectGroupState::PeakingFilterEffectGroupState()
        : m_oldCenter(0.022675737) {
    m_filter = new EngineFilterBiquad1Peaking(1, 0.022675737, 1); 
}

PeakingFilterEffectGroupState::~PeakingFilterEffectGroupState() {
    delete m_filter;
}

PeakingFilterEffect::PeakingFilterEffect(EngineEffect* pEffect,
                                 const EffectManifest& manifest) {
    Q_UNUSED(manifest);
    m_pPotQ = pEffect->getParameterById("q");
    m_pPotGain = pEffect->getParameterById("gain");
    m_pPotCenter = pEffect->getParameterById("center");
}

PeakingFilterEffect::~PeakingFilterEffect() {
}

void PeakingFilterEffect::processChannel(
        const ChannelHandle& handle,
        PeakingFilterEffectGroupState* channelState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const EffectProcessor::EnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);


    double q = m_pPotQ->value();
    double gain = m_pPotGain->value();
    double center = m_pPotCenter->value();

    if (q != channelState->m_oldQ ||
            gain != channelState->m_oldGain ||
            center != channelState->m_oldCenter) {
        channelState->m_filter->setFrequencyCorners(1, center, q, gain);
    }

    if (gain) {
        channelState->m_filter->process(pInput, pOutput, numSamples);
    } else {
        memcpy(pOutput, pInput, numSamples * sizeof(CSAMPLE));
        channelState->m_filter->pauseFilter();
    }

    for (unsigned int i = 0; i < numSamples; i++) {
        pOutput[i] = tanhf(pOutput[i]); 
    }

    channelState->m_oldQ = q;
    channelState->m_oldGain = gain;
    channelState->m_oldCenter = center;
}
