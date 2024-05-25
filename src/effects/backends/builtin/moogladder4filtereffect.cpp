#include "effects/backends/builtin/moogladder4filtereffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefiltermoogladder4.h"

static constexpr double kMinCorner = 0.0003; // 13 Hz @ 44100
static constexpr double kMaxCorner = 0.5;    // 22050 Hz @ 44100

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
    pManifest->setDescription(
            QObject::tr("A 4-pole Moog ladder filter, based on Antti "
                        "Houvilainen's non linear digital implementation"));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.5);

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    lpf->setDefaultLinkType(EffectManifestParameter::LinkType::LinkedLeft);
    lpf->setNeutralPointOnScale(1);
    lpf->setRange(kMinCorner, kMaxCorner, kMaxCorner);

    EffectManifestParameterPointer q = pManifest->addParameter();
    q->setId("resonance");
    q->setName(QObject::tr("Resonance"));
    q->setShortName(QObject::tr("Res"));
    q->setDescription(QObject::tr("Resonance of the filters. 4 = self oscillating"));
    q->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    q->setUnitsHint(EffectManifestParameter::UnitsHint::SampleRate);
    q->setRange(0.0, 1.0, 4.0);

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    hpf->setDefaultLinkType(EffectManifestParameter::LinkType::LinkedRight);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setRange(kMinCorner, kMinCorner, kMaxCorner);

    return pManifest;
}

MoogLadder4FilterGroupState::MoogLadder4FilterGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_loFreq(kMaxCorner),
          m_resonance(0),
          m_hiFreq(kMinCorner),
          m_samplerate(engineParameters.sampleRate()) {
    m_pBuf = SampleUtil::alloc(engineParameters.samplesPerBuffer());
    m_pLowFilter = new EngineFilterMoogLadder4Low(
            engineParameters.sampleRate(),
            m_loFreq * engineParameters.sampleRate(),
            m_resonance);
    m_pHighFilter = new EngineFilterMoogLadder4High(
            engineParameters.sampleRate(),
            m_hiFreq * engineParameters.sampleRate(),
            m_resonance);
}

MoogLadder4FilterGroupState::~MoogLadder4FilterGroupState() {
    SampleUtil::free(m_pBuf);
    delete m_pLowFilter;
    delete m_pHighFilter;
}

void MoogLadder4FilterEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pLPF = parameters.value("lpf");
    m_pResonance = parameters.value("resonance");
    m_pHPF = parameters.value("hpf");
}

void MoogLadder4FilterEffect::processChannel(
        MoogLadder4FilterGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
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
            pState->m_samplerate != engineParameters.sampleRate()) {
        pState->m_pLowFilter->setParameter(engineParameters.sampleRate(),
                static_cast<float>(lpf * engineParameters.sampleRate()),
                static_cast<float>(resonance));
    }

    if (pState->m_hiFreq != hpf ||
            pState->m_resonance != resonance ||
            pState->m_samplerate != engineParameters.sampleRate()) {
        pState->m_pHighFilter->setParameter(engineParameters.sampleRate(),
                static_cast<float>(hpf * engineParameters.sampleRate()),
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
        pState->m_pHighFilter->process(pInput, pHpfOutput, engineParameters.samplesPerBuffer());
    } else if (pState->m_hiFreq > kMinCorner) {
        // hpf disabling
        pState->m_pHighFilter->processAndPauseFilter(pInput,
                pHpfOutput,
                engineParameters.samplesPerBuffer());
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < kMaxCorner) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, engineParameters.samplesPerBuffer());
    } else if (pState->m_loFreq < kMaxCorner) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput,
                engineParameters.samplesPerBuffer());
    } else if (pLpfInput == pInput) {
        // Both disabled
        if (pOutput != pInput) {
            // We need to copy pInput pOutput
            SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        }
    }

    pState->m_loFreq = lpf;
    pState->m_resonance = resonance;
    pState->m_hiFreq = hpf;
    pState->m_samplerate = engineParameters.sampleRate();
}
