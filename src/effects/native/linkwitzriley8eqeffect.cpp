#include "effects/native/linkwitzriley8eqeffect.h"
#include "util/math.h"

static const unsigned int kStartupSamplerate = 44100;
static const unsigned int kStartupLoFreq = 246;
static const unsigned int kStartupHiFreq = 2484;

// static
QString LinkwitzRiley8EQEffect::getId() {
    return "org.mixxx.effects.linkwitzrileyeq";
}

// static
EffectManifest LinkwitzRiley8EQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("LinkwitzRiley8 EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Linkwitz-Riley 8th order filter equalizer (optimized crossover, constant phase shift, roll-off -48 db/Oct). "
        "To adjust frequency shelves see the Equalizer preferences."));
    manifest.setIsMixingEQ(true); 

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

    EffectManifestParameter* killLow = manifest.addParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription(QObject::tr("Kill the Low Filter"));
    killLow->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killLow->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killLow->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killLow->setDefault(0);
    killLow->setMinimum(0);
    killLow->setMaximum(1);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription(QObject::tr("Gain for Band Filter"));
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setNeutralPointOnScale(0.5);    
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(4.0);

    EffectManifestParameter* killMid = manifest.addParameter();
    killMid->setId("killMid");
    killMid->setName(QObject::tr("Kill Mid"));
    killMid->setDescription(QObject::tr("Kill the Mid Filter"));
    killMid->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killMid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killMid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killMid->setDefault(0);
    killMid->setMinimum(0);
    killMid->setMaximum(1);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("high");
    high->setName(QObject::tr("High"));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setNeutralPointOnScale(0.5);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    EffectManifestParameter* killHigh = manifest.addParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription(QObject::tr("Kill the High Filter"));
    killHigh->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killHigh->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killHigh->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killHigh->setDefault(0);
    killHigh->setMinimum(0);
    killHigh->setMaximum(1);

    return manifest;
}

LinkwitzRiley8EQEffectGroupState::LinkwitzRiley8EQEffectGroupState()
        : old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_oldSampleRate(kStartupSamplerate),
          m_loFreq(kStartupLoFreq),
          m_hiFreq(kStartupHiFreq) {

    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBandBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    m_low1 = new EngineFilterLinkwtzRiley8Low(kStartupSamplerate, kStartupLoFreq);
    m_high1 = new EngineFilterLinkwtzRiley8High(kStartupSamplerate, kStartupLoFreq);
    m_low2 = new EngineFilterLinkwtzRiley8Low(kStartupSamplerate, kStartupHiFreq);
    m_high2 = new EngineFilterLinkwtzRiley8High(kStartupSamplerate, kStartupHiFreq);
}

LinkwitzRiley8EQEffectGroupState::~LinkwitzRiley8EQEffectGroupState() {
    delete m_low1;
    delete m_high1;
    delete m_low2;
    delete m_high2;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pBandBuf);
    SampleUtil::free(m_pHighBuf);
}

void LinkwitzRiley8EQEffectGroupState::setFilters(int sampleRate, int lowFreq,
                                               int highFreq) {
    m_low1->setFrequencyCorners(sampleRate, lowFreq);
    m_high1->setFrequencyCorners(sampleRate, lowFreq);
    m_low2->setFrequencyCorners(sampleRate, highFreq);
    m_high2->setFrequencyCorners(sampleRate, highFreq);
}

LinkwitzRiley8EQEffect::LinkwitzRiley8EQEffect(EngineEffect* pEffect,
                                         const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getParameterById("killLow")),
          m_pKillMid(pEffect->getParameterById("killMid")),
          m_pKillHigh(pEffect->getParameterById("killHigh")) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
}

LinkwitzRiley8EQEffect::~LinkwitzRiley8EQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void LinkwitzRiley8EQEffect::processGroup(const QString& group,
        LinkwitzRiley8EQEffectGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const EffectProcessor::EnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    if (!m_pKillLow->toBool()) {
        fLow = m_pPotLow->value();
    }
    if (!m_pKillMid->toBool()) {
        fMid = m_pPotMid->value();
    }
    if (!m_pKillHigh->toBool()) {
        fHigh = m_pPotHigh->value();
    }

    if (pState->m_oldSampleRate != sampleRate ||
            (pState->m_loFreq != static_cast<int>(m_pLoFreqCorner->get())) ||
            (pState->m_hiFreq != static_cast<int>(m_pHiFreqCorner->get()))) {
        pState->m_loFreq = static_cast<int>(m_pLoFreqCorner->get());
        pState->m_hiFreq = static_cast<int>(m_pHiFreqCorner->get());
        pState->m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate, pState->m_loFreq, pState->m_hiFreq);
    }

    pState->m_high2->process(pInput, pState->m_pHighBuf, numSamples); // HighPass first run
    pState->m_low2->process(pInput, pState->m_pLowBuf, numSamples); // LowPass first run for low and bandpass

    if (fMid != pState->old_mid ||
            fHigh != pState->old_high) {
        SampleUtil::copy2WithRampingGain(pState->m_pHighBuf,
                pState->m_pHighBuf, pState->old_high, fHigh,
                pState->m_pLowBuf, pState->old_mid, fMid,
                numSamples);
    } else {
        SampleUtil::copy2WithGain(pState->m_pHighBuf,
                pState->m_pHighBuf, fHigh,
                pState->m_pLowBuf, fMid,
                numSamples);
    }

    pState->m_high1->process(pState->m_pHighBuf, pState->m_pBandBuf, numSamples); // HighPass + BandPass second run
    pState->m_low1->process(pState->m_pLowBuf, pState->m_pLowBuf, numSamples); // LowPass second run

    if (fLow != pState->old_low) {
        SampleUtil::copy2WithRampingGain(pOutput,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf, 1, 1,
                numSamples);
    } else {
        SampleUtil::copy2WithGain(pOutput,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf, 1,
                numSamples);
    }

    if (enableState == EffectProcessor::DISABLING) {
        // we rely on the ramping to dry in EngineEffect
        // since this EQ is not fully dry at unity
        pState->m_low1->pauseFilter();
        pState->m_low2->pauseFilter();
        pState->m_high1->pauseFilter();
        pState->m_high2->pauseFilter();
        pState->old_low = 1.0;
        pState->old_mid = 1.0;
        pState->old_high = 1.0;
    } else {
        pState->old_low = fLow;
        pState->old_mid = fMid;
        pState->old_high = fHigh;
    }
}
