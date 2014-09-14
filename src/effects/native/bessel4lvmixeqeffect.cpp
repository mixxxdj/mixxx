#include "effects/native/bessel4lvmixeqeffect.h"
#include "util/math.h"

static const unsigned int kStartupSamplerate = 44100;
static const unsigned int kStartupLoFreq = 246;
static const unsigned int kStartupHiFreq = 2484;

// constant to calculate the group delay from the low pass corner
// mean value of a set of fid_calc_delay() calls for different corners
static const double kGroupDelay1Hz = 0.5067964223;
// kDelayOffset is required to match short delays.
static const double kDelayOffset = 0.2;
static const double kMaxCornerFreq = 14212;
static const int kMaxDelay = 3300; // allows a 30 Hz filter at 97346;

// static
QString Bessel4LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel4lvmixeq";
}

// static
EffectManifest Bessel4LVMixEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Bessel4 LV-Mix EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Bessel 4th order filter equalizer with Lipshitz and Vanderkooy mix (bit perfect unity, roll-off -24 db/Oct). "
        "To adjust frequency shelves see the Equalizer preferences."));

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription(QObject::tr("Gain for Band Filter"));
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(4.0);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("high");
    high->setName(QObject::tr("High"));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    return manifest;
}

Bessel4LVMixEQEffectGroupState::Bessel4LVMixEQEffectGroupState()
        : old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_oldSampleRate(kStartupSamplerate),
          m_loFreq(kStartupLoFreq), 
          m_hiFreq(kStartupHiFreq) {
    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBandBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    m_low1 = new EngineFilterBessel4Low(kStartupSamplerate, kStartupLoFreq);
    m_low2 = new EngineFilterBessel4Low(kStartupSamplerate, kStartupHiFreq);
    m_delay2 = new EngineFilterDelay<kMaxDelay>();
    m_delay3 = new EngineFilterDelay<kMaxDelay>();
    setFilters(kStartupSamplerate, kStartupLoFreq, kStartupHiFreq);
}

Bessel4LVMixEQEffectGroupState::~Bessel4LVMixEQEffectGroupState() {
    delete m_low1;
    delete m_low2;
    delete m_delay2;
    delete m_delay3;
    SampleUtil::free(m_pLowBuf);
    SampleUtil::free(m_pBandBuf);
    SampleUtil::free(m_pHighBuf);
}

void Bessel4LVMixEQEffectGroupState::setFilters(int sampleRate, int lowFreq,
                                                int highFreq) {
    double delayLow1 = sampleRate * kGroupDelay1Hz / lowFreq + kDelayOffset;
    delayLow1 = delayLow1 / 3 * 2;
    double delayLow2 = sampleRate * kGroupDelay1Hz / highFreq + kDelayOffset;
    delayLow2 = delayLow2 / 3 * 2;
    // Since we delay only full samples, we can only allow frequencies
    // producing such delays
    unsigned int iDelayLow1 = (unsigned int)delayLow1;
    unsigned int iDelayLow2 = (unsigned int)delayLow2;
    int stepLowFreq;
    if (iDelayLow1 > 1) {
        stepLowFreq = sampleRate * kGroupDelay1Hz / (iDelayLow1 - kDelayOffset + 0.5);
    } else {
        iDelayLow1 = 1;
        stepLowFreq = kMaxCornerFreq;
    }
    int stepHighFreq;
    if (iDelayLow1 > 1) {
        stepHighFreq = sampleRate * kGroupDelay1Hz / (iDelayLow2 - kDelayOffset + 0.5);
    } else {
        iDelayLow2 = 1;
        stepHighFreq = kMaxCornerFreq;
    }

    m_low1->setFrequencyCorners(sampleRate, stepLowFreq);
    m_low2->setFrequencyCorners(sampleRate, stepHighFreq);
    m_delay2->setDelay((iDelayLow1 - iDelayLow2) * 2);
    m_delay3->setDelay(iDelayLow1 * 2);
}

Bessel4LVMixEQEffect::Bessel4LVMixEQEffect(EngineEffect* pEffect,
                                           const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
}

Bessel4LVMixEQEffect::~Bessel4LVMixEQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void Bessel4LVMixEQEffect::processGroup(const QString& group,
                                        Bessel4LVMixEQEffectGroupState* pState,
                                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                                        const unsigned int numSamples,
                                        const unsigned int sampleRate,
                                        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    fLow = m_pPotLow->value().toDouble();
    fMid = m_pPotMid->value().toDouble();
    fHigh = m_pPotHigh->value().toDouble();

    if (pState->m_oldSampleRate != sampleRate ||
            (pState->m_loFreq != static_cast<int>(m_pLoFreqCorner->get())) ||
            (pState->m_hiFreq != static_cast<int>(m_pHiFreqCorner->get()))) {
        pState->m_loFreq = static_cast<int>(m_pLoFreqCorner->get());
        pState->m_hiFreq = static_cast<int>(m_pHiFreqCorner->get());
        pState->m_oldSampleRate = sampleRate;
        // Clamp frequency corners to the border, defined by the maximum delay.
        int minFreq = sampleRate / (kMaxDelay - 1) * kGroupDelay1Hz * 2;
        pState->setFilters(sampleRate,
                math_max(pState->m_loFreq, minFreq),
                math_max(pState->m_hiFreq, minFreq));
    }

    // Since a Bessel Low pass Filter has a constant group delay in the pass band,
    // we can subtract or add the filtered signal to the dry signal if we compensate this delay
    // The dry signal represents the high gain
    // Then the higher low pass is added and at least the lower low pass result.
    fLow = fLow - fMid;
    fMid = fMid - fHigh;

    if (fHigh || pState->old_high) {
        pState->m_delay3->process(pInput, pState->m_pHighBuf, numSamples);
    } else {
        pState->m_delay3->pauseFilter();
    }

    if (fMid || pState->old_mid) {
        pState->m_low2->process(pInput, pState->m_pBandBuf, numSamples);
        pState->m_delay2->process(pState->m_pBandBuf, pState->m_pBandBuf, numSamples);
    } else {
        pState->m_low2->pauseFilter();
        pState->m_delay2->pauseFilter();
    }

    if (fLow || pState->old_low) {
        pState->m_low1->process(pInput, pState->m_pLowBuf, numSamples);
    } else {
        pState->m_low1->pauseFilter();
    }

    // Test code for comparing streams as two stereo channels
    //for (unsigned int i = 0; i < numSamples; i +=2) {
    //    pOutput[i] = pState->m_pLowBuf[i];
    //    pOutput[i + 1] = pState->m_pBandBuf[i];
    //}

    if (fLow != pState->old_low ||
            fMid != pState->old_mid ||
            fHigh != pState->old_high) {
        SampleUtil::copy3WithRampingGain(pOutput,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf, pState->old_mid, fMid,
                pState->m_pHighBuf, pState->old_high, fHigh,
                numSamples);
    } else {
        SampleUtil::copy3WithGain(pOutput,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf, fMid,
                pState->m_pHighBuf, fHigh,
                numSamples);
    }

    pState->old_low = fLow;
    pState->old_mid = fMid;
    pState->old_high = fHigh;
}
