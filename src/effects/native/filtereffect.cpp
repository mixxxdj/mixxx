#include "effects/native/filtereffect.h"
#include "util/math.h"

static const unsigned int kStartupSamplerate = 44100;

// static
QString FilterEffect::getId() {
    return "org.mixxx.effects.filter";
}

// static
EffectManifest FilterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Filter"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("Allows to fade a song out by sweeping a low or high pass filter");

    EffectManifestParameter* hpf = manifest.addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription("Corner frequency ratio of the high pass filter");
    hpf->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    hpf->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    hpf->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    hpf->setDefaultLinkType(EffectManifestParameter::LINK_LINKED_RIGHT);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(0.0);
    hpf->setMinimum(0.0003);
    hpf->setMaximum(0.5);

    EffectManifestParameter* q = manifest.addParameter();
    q->setId("q");
    q->setName(QObject::tr("Q"));
    q->setDescription("Resonance of the filters, 0.707 = Flat top");
    q->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    q->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    q->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    q->setDefault(2); // 0,707
    q->setMinimum(0.1);
    q->setMaximum(4.0);

    EffectManifestParameter* lpf = manifest.addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription("Corner frequency ratio of the low pass filter");
    lpf->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    lpf->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    lpf->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    lpf->setDefaultLinkType(EffectManifestParameter::LINK_LINKED_LEFT);
    lpf->setNeutralPointOnScale(1);
    lpf->setDefault(0.2);
    lpf->setMinimum(0.0003);
    lpf->setMaximum(0.5);

    return manifest;
}

FilterGroupState::FilterGroupState()
        : m_hiFreq(0),
          m_q(0.707),
          m_loFreq(0.5) {
    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighFilter = new EngineFilterBiquad1High(1, m_hiFreq, m_q);
    m_pLowFilter = new EngineFilterBiquad1Low(1, m_loFreq, m_q);
}

FilterGroupState::~FilterGroupState() {
    SampleUtil::free(m_pBuf );
    delete m_pHighFilter;
    delete m_pLowFilter;
}

FilterEffect::FilterEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pHPF(pEffect->getParameterById("hpf")),
          m_pQ(pEffect->getParameterById("q")),
          m_pLPF(pEffect->getParameterById("lpf")) {
    Q_UNUSED(manifest);
}

FilterEffect::~FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

void FilterEffect::processGroup(const QString& group,
                                FilterGroupState* pState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);


    double hpf = m_pHPF->value().toDouble();
    double q = m_pQ->value().toDouble();
    double lpf = m_pLPF->value().toDouble();

    if ((pState->m_hiFreq != hpf) ||
            (pState->m_q != q)) {
        pState->m_pHighFilter->setFrequencyCorners(1, hpf, q);
    }

    if ((pState->m_loFreq != lpf) ||
            (pState->m_q != q)) {
        pState->m_pLowFilter->setFrequencyCorners(1, lpf, q);
    }

    if (hpf) {
        if (lpf < 0.5) {
            pState->m_pLowFilter->process(pInput, pState->m_pBuf, numSamples);
            pState->m_pHighFilter->process(pState->m_pBuf, pOutput, numSamples);
        } else {
            pState->m_pLowFilter->pauseFilter();
            pState->m_pHighFilter->process(pInput, pOutput, numSamples);
        }
    } else {
        pState->m_pHighFilter->pauseFilter();
        if (lpf < 0.5) {
            pState->m_pHighFilter->process(pInput, pOutput, numSamples);
        } else {
            pState->m_pLowFilter->pauseFilter();
            SampleUtil::copyWithGain(pOutput, pInput, 1.0, numSamples);
        }
    }

    pState->m_hiFreq = lpf;
    pState->m_q = q;
    pState->m_loFreq = hpf;
}
