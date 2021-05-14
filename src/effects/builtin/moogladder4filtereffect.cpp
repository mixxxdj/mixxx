#include "effects/builtin/moogladder4filtereffect.h"
#include "util/math.h"


static const double kMinCorner = 0.0003; // 13 Hz @ 44100
static const double kMaxCorner = 0.5; // 22050 Hz @ 44100

// static
QString MoogLadder4FilterEffect::getId() {
    return "org.mixxx.effects.moogladder4filter";
}

// static
EffectManifestPointer MoogLadder4FilterEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Moog Ladder 4 Filter"));
    pManifest->setShortName(QObject::tr("Moog Filter"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "A 4-pole Moog ladder filter, based on Antti Houvilainen's non linear digital implementation"));
    pManifest->setEffectRampsFromDry(true);

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_LEFT);
    lpf->setNeutralPointOnScale(1);
    lpf->setDefault(kMaxCorner);
    lpf->setMinimum(kMinCorner);
    lpf->setMaximum(kMaxCorner);

    EffectManifestParameterPointer q = pManifest->addParameter();
    q->setId("resonance");
    q->setName(QObject::tr("Resonance"));
    q->setShortName(QObject::tr("Res"));
    q->setDescription(QObject::tr("Resonance of the filters. 4 = self oscillating"));
    q->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    q->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UnitsHint::SAMPLERATE);
    q->setMinimum(0.0);
    q->setMaximum(4.0);
    q->setDefault(1.0);

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    hpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_RIGHT);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(kMinCorner);
    hpf->setMinimum(kMinCorner);
    hpf->setMaximum(kMaxCorner);

    return pManifest;
}

MoogLadder4FilterGroupState::MoogLadder4FilterGroupState(
        const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_loFreq(kMaxCorner),
          m_resonance(0),
          m_hiFreq(kMinCorner),
          m_samplerate(bufferParameters.sampleRate()) {
    m_pBuf = SampleUtil::alloc(bufferParameters.samplesPerBuffer());
    m_pLowFilter = new EngineFilterMoogLadder4Low(
            bufferParameters.sampleRate(),
            m_loFreq * bufferParameters.sampleRate(), m_resonance);
    m_pHighFilter = new EngineFilterMoogLadder4High(
            bufferParameters.sampleRate(),
            m_hiFreq * bufferParameters.sampleRate(), m_resonance);
}

MoogLadder4FilterGroupState::~MoogLadder4FilterGroupState() {
    SampleUtil::free(m_pBuf);
    delete m_pLowFilter;
    delete m_pHighFilter;
}

MoogLadder4FilterEffect::MoogLadder4FilterEffect(EngineEffect* pEffect)
        : m_pLPF(pEffect->getParameterById("lpf")),
          m_pResonance(pEffect->getParameterById("resonance")),
          m_pHPF(pEffect->getParameterById("hpf")) {
}

MoogLadder4FilterEffect::~MoogLadder4FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

void MoogLadder4FilterEffect::processChannel(
        const ChannelHandle& handle,
        MoogLadder4FilterGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    double resonance = m_pResonance->value();
    double hpf;
    double lpf;
    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        hpf = kMinCorner;
        lpf = kMaxCorner;
    } else {
        hpf = m_pHPF->value();
        lpf = m_pLPF->value();
    }

    if (pState->m_loFreq != lpf ||
            pState->m_resonance != resonance ||
            pState->m_samplerate != bufferParameters.sampleRate()) {
        pState->m_pLowFilter->setParameter(bufferParameters.sampleRate(),
                static_cast<float>(lpf * bufferParameters.sampleRate()),
                static_cast<float>(resonance));
    }

    if (pState->m_hiFreq != hpf ||
            pState->m_resonance != resonance ||
            pState->m_samplerate != bufferParameters.sampleRate()) {
        pState->m_pHighFilter->setParameter(bufferParameters.sampleRate(),
                static_cast<float>(hpf * bufferParameters.sampleRate()),
                static_cast<float>(resonance));
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
        pState->m_pHighFilter->process(pInput, pHpfOutput, bufferParameters.samplesPerBuffer());
    } else if (pState->m_hiFreq > kMinCorner) {
        // hpf disabling
        pState->m_pHighFilter->processAndPauseFilter(pInput,
                pHpfOutput, bufferParameters.samplesPerBuffer());
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < kMaxCorner) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, bufferParameters.samplesPerBuffer());
    } else if (pState->m_loFreq < kMaxCorner) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput, bufferParameters.samplesPerBuffer());
    } else if (pLpfInput == pInput) {
        // Both disabled
        if (pOutput != pInput) {
            // We need to copy pInput pOutput
            SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());
        }
    }

    pState->m_loFreq = lpf;
    pState->m_resonance = resonance;
    pState->m_hiFreq = hpf;
    pState->m_samplerate = bufferParameters.sampleRate();
}
