#include "effects/builtin/linkwitzriley8eqeffect.h"

#include "effects/builtin/equalizer_util.h"
#include "util/math.h"

static const unsigned int kStartupSamplerate = 44100;
static const unsigned int kStartupLoFreq = 246;
static const unsigned int kStartupHiFreq = 2484;

// static
QString LinkwitzRiley8EQEffect::getId() {
    return "org.mixxx.effects.linkwitzrileyeq";
}

// static
EffectManifestPointer LinkwitzRiley8EQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("LinkwitzRiley8 Isolator"));
    pManifest->setShortName(QObject::tr("LR8 ISO"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
        "A Linkwitz-Riley 8th-order filter isolator (optimized crossover, constant phase shift, roll-off -48 dB/octave).") + " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setIsMixingEQ(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), false);
    return pManifest;
}

LinkwitzRiley8EQEffectGroupState::LinkwitzRiley8EQEffectGroupState(
        const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_oldSampleRate(kStartupSamplerate),
          m_loFreq(kStartupLoFreq),
          m_hiFreq(kStartupHiFreq) {

    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMidBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    m_low1 = new EngineFilterLinkwitzRiley8Low(kStartupSamplerate, kStartupLoFreq);
    m_high1 = new EngineFilterLinkwitzRiley8High(kStartupSamplerate, kStartupLoFreq);
    m_low2 = new EngineFilterLinkwitzRiley8Low(kStartupSamplerate, kStartupHiFreq);
    m_high2 = new EngineFilterLinkwitzRiley8High(kStartupSamplerate, kStartupHiFreq);
}

LinkwitzRiley8EQEffectGroupState::~LinkwitzRiley8EQEffectGroupState() {
    delete m_low1;
    delete m_high1;
    delete m_low2;
    delete m_high2;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pMidBuf);
    SampleUtil::free(m_pHighBuf);
}

void LinkwitzRiley8EQEffectGroupState::setFilters(int sampleRate, int lowFreq,
                                               int highFreq) {
    m_low1->setFrequencyCorners(sampleRate, lowFreq);
    m_high1->setFrequencyCorners(sampleRate, lowFreq);
    m_low2->setFrequencyCorners(sampleRate, highFreq);
    m_high2->setFrequencyCorners(sampleRate, highFreq);
}

LinkwitzRiley8EQEffect::LinkwitzRiley8EQEffect(EngineEffect* pEffect)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getParameterById("killLow")),
          m_pKillMid(pEffect->getParameterById("killMid")),
          m_pKillHigh(pEffect->getParameterById("killHigh")) {
    m_pLoFreqCorner = new ControlProxy("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlProxy("[Mixer Profile]", "HiEQFrequency");
}

LinkwitzRiley8EQEffect::~LinkwitzRiley8EQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void LinkwitzRiley8EQEffect::processChannel(const ChannelHandle& handle,
                                            LinkwitzRiley8EQEffectGroupState* pState,
                                            const CSAMPLE* pInput, CSAMPLE* pOutput,
                                            const mixxx::EngineParameters& bufferParameters,
                                            const EffectEnableState enableState,
                                            const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    if (!m_pKillLow->toBool()) {
        fLow = static_cast<float>(m_pPotLow->value());
    }
    if (!m_pKillMid->toBool()) {
        fMid = static_cast<float>(m_pPotMid->value());
    }
    if (!m_pKillHigh->toBool()) {
        fHigh = static_cast<float>(m_pPotHigh->value());
    }

    if (pState->m_oldSampleRate != bufferParameters.sampleRate() ||
            (pState->m_loFreq != static_cast<int>(m_pLoFreqCorner->get())) ||
            (pState->m_hiFreq != static_cast<int>(m_pHiFreqCorner->get()))) {
        pState->m_loFreq = static_cast<int>(m_pLoFreqCorner->get());
        pState->m_hiFreq = static_cast<int>(m_pHiFreqCorner->get());
        pState->m_oldSampleRate = bufferParameters.sampleRate();
        pState->setFilters(bufferParameters.sampleRate(), pState->m_loFreq, pState->m_hiFreq);
    }

    pState->m_high2->process(pInput, pState->m_pHighBuf, bufferParameters.samplesPerBuffer()); // HighPass first run
    pState->m_low2->process(pInput, pState->m_pLowBuf, bufferParameters.samplesPerBuffer()); // LowPass first run for low and bandpass

    if (fMid != pState->old_mid || fHigh != pState->old_high) {
        SampleUtil::applyRampingGain(pState->m_pHighBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_high),
                fHigh,
                bufferParameters.samplesPerBuffer());
        SampleUtil::addWithRampingGain(pState->m_pHighBuf,
                pState->m_pLowBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_mid),
                fMid,
                bufferParameters.samplesPerBuffer());
    } else {
        SampleUtil::applyGain(pState->m_pHighBuf, fHigh, bufferParameters.samplesPerBuffer());
        SampleUtil::addWithGain(pState->m_pHighBuf,
                                pState->m_pLowBuf, fMid,
                                bufferParameters.samplesPerBuffer());
    }

    pState->m_high1->process(pState->m_pHighBuf, pState->m_pMidBuf, bufferParameters.samplesPerBuffer()); // HighPass + BandPass second run
    pState->m_low1->process(pState->m_pLowBuf, pState->m_pLowBuf, bufferParameters.samplesPerBuffer()); // LowPass second run

    if (fLow != pState->old_low) {
        SampleUtil::copy2WithRampingGain(pOutput,
                pState->m_pLowBuf,
                static_cast<CSAMPLE_GAIN>(pState->old_low),
                fLow,
                pState->m_pMidBuf,
                1,
                1,
                bufferParameters.samplesPerBuffer());
    } else {
        SampleUtil::copy2WithGain(pOutput,
                pState->m_pLowBuf, fLow,
                pState->m_pMidBuf, 1,
                bufferParameters.samplesPerBuffer());
    }

    if (enableState == EffectEnableState::Disabling) {
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
