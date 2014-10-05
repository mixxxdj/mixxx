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
    q->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    q->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    q->setDefault(0);
    q->setMinimum(0.1);
    q->setMaximum(10);

    EffectManifestParameter* gain = manifest.addParameter();
    gain->setId(QString("gain"));
    gain->setName(QString("Gain"));
    gain->setDescription(QString("Gain"));
    gain->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    gain->setValueHint(EffectManifestParameter::VALUE_FLOAT);
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
    center->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    center->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    center->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    center->setDefault(0.022675737); // 1000 Hz @ 44100
    center->setMinimum(0.003);
    center->setMaximum(0.5);

    return manifest;
}

PeakingFilterEffectGroupState::PeakingFilterEffectGroupState() {
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

void PeakingFilterEffect::processGroup(const QString& group,
                                   PeakingFilterEffectGroupState* pState,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const unsigned int numSamples,
                                   const unsigned int sampleRate,
                                   const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);


    float q = m_pPotQ->value().toDouble();
    float gain = m_pPotGain->value().toDouble();
    float center = m_pPotCenter->value().toDouble();;

    if (q != pState->m_oldQ ||
            gain != pState->m_oldGain ||
            center != pState->m_oldCenter) {
        pState->m_filter->setFrequencyCorners(1, center, q, gain); 
    }

    if (gain) {
        pState->m_filter->process(pInput, pOutput, numSamples);
    } else {
        memcpy(pOutput, pInput, numSamples * sizeof(CSAMPLE));
        pState->m_filter->pauseFilter();
    }

    pState->m_oldQ = q; 
    pState->m_oldGain = gain; 
    pState->m_oldCenter = center; 
}
