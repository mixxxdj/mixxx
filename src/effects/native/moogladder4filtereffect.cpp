#include "effects/native/moogladder4filtereffect.h"
#include "util/math.h"


static const double kMinCorner = 0.0003; // 13 Hz @ 44100
static const double kMaxCorner = 0.5; // 22050 Hz @ 44100
static const unsigned int kStartupSamplerate = 44100;

// static
QString MoogLadder4FilterEffect::getId() {
    return "org.mixxx.effects.moogladder4filter";
}

// static
EffectManifest MoogLadder4FilterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Moog Ladder 4 Filter"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
            "A 4-pole Moog ladder filter, based on Antti Houvilainen's non linear digital implementation"));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsForFilterKnob(true);

    EffectManifestParameter* lpf = manifest.addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    lpf->setDefaultLinkType(EffectManifestParameter::LINK_LINKED_LEFT);
    lpf->setNeutralPointOnScale(1);
    lpf->setDefault(kMaxCorner);
    lpf->setMinimum(kMinCorner);
    lpf->setMaximum(kMaxCorner);

    EffectManifestParameter* q = manifest.addParameter();
    q->setId("resonance");
    q->setName(QObject::tr("Resonance"));
    q->setDescription(QObject::tr("Resonance of the filters. 4 = self oscillating"));
    q->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    q->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UNITS_SAMPLERATE);
    q->setDefault(0);
    q->setMinimum(0.0);
    q->setMaximum(4.0);

    EffectManifestParameter* hpf = manifest.addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    hpf->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    hpf->setDefaultLinkType(EffectManifestParameter::LINK_LINKED_RIGHT);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(kMinCorner);
    hpf->setMinimum(kMinCorner);
    hpf->setMaximum(kMaxCorner);

    return manifest;
}

MoogLadder4FilterGroupState::MoogLadder4FilterGroupState()
        : m_loFreq(kMaxCorner),
          m_resonance(0),
          m_hiFreq(kMinCorner),
          m_samplerate(kStartupSamplerate) {
    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pLowFilter = new EngineFilterMoogLadder4Low(
            kStartupSamplerate, m_loFreq * kStartupSamplerate, m_resonance);
    m_pHighFilter = new EngineFilterMoogLadder4High(
            kStartupSamplerate, m_hiFreq * kStartupSamplerate, m_resonance);
}

MoogLadder4FilterGroupState::~MoogLadder4FilterGroupState() {
    SampleUtil::free(m_pBuf);
    delete m_pLowFilter;
    delete m_pHighFilter;
}

MoogLadder4FilterEffect::MoogLadder4FilterEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pLPF(pEffect->getParameterById("lpf")),
          m_pResonance(pEffect->getParameterById("resonance")),
          m_pHPF(pEffect->getParameterById("hpf")) {
    Q_UNUSED(manifest);
}

MoogLadder4FilterEffect::~MoogLadder4FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

void MoogLadder4FilterEffect::processGroup(const QString& group,
        MoogLadder4FilterGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const EffectProcessor::EnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);


    double resonance = m_pResonance->value();
    double hpf;
    double lpf;
    if (enableState == EffectProcessor::DISABLING) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        hpf = kMinCorner;
        lpf = kMaxCorner;
    } else {
        hpf = m_pHPF->value();
        lpf = m_pLPF->value();
    }

    if (pState->m_loFreq != lpf ||
            pState->m_resonance != resonance ||
            pState->m_samplerate != sampleRate) {
        pState->m_pLowFilter->setParameter(
                sampleRate, lpf * sampleRate, resonance);
    }

    if (pState->m_hiFreq != hpf ||
            pState->m_resonance != resonance ||
            pState->m_samplerate != sampleRate) {
        pState->m_pHighFilter->setParameter(
                sampleRate, hpf * sampleRate, resonance);
    }

    const CSAMPLE* pLpfInput = pState->m_pBuf;
    CSAMPLE* pHpfOutput = pState->m_pBuf;
    if (lpf >= kMaxCorner && pState->m_loFreq >= kMaxCorner) {
        // Lpf disabled Hpf can write directly to output
        pHpfOutput = pOutput;
        pLpfInput = pHpfOutput;
    }

    if (hpf > kMinCorner) {
        // hpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pHighFilter->process(pInput, pHpfOutput, numSamples);
    } else if (pState->m_hiFreq > kMinCorner) {
            // hpf disabling
            pState->m_pHighFilter->processAndPauseFilter(pInput,
                    pHpfOutput, numSamples);
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < kMaxCorner) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, numSamples);
    } else if (pState->m_loFreq < kMaxCorner) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput, numSamples);
    } else if (pLpfInput == pInput) {
        // Both disabled
        SampleUtil::copyWithGain(pOutput, pInput, 1.0, numSamples);
    }

    pState->m_loFreq = lpf;
    pState->m_resonance = resonance;
    pState->m_hiFreq = hpf;
    pState->m_samplerate = sampleRate;
}
