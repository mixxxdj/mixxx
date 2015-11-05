#include "effects/native/bessel4lvmixeqeffect.h"
#include "util/math.h"

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
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
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
    mid->setNeutralPointOnScale(0.5);
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
    high->setNeutralPointOnScale(0.5);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    return manifest;
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
                                        const EffectProcessor::EnableState enableState,
                                        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    double fLow;
    double fMid;
    double fHigh;
    if (enableState == EffectProcessor::DISABLING) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLow = 1.0;
        fMid = 1.0;
        fHigh = 1.0;
    } else {
        fLow = m_pPotLow->value().toDouble();
        fMid = m_pPotMid->value().toDouble();
        fHigh = m_pPotHigh->value().toDouble();
    }

    if (pState->m_oldSampleRate != sampleRate ||
            (pState->m_loFreq != m_pLoFreqCorner->get()) ||
            (pState->m_hiFreq != m_pHiFreqCorner->get())) {
        pState->m_loFreq = m_pLoFreqCorner->get();
        pState->m_hiFreq = m_pHiFreqCorner->get();
        pState->m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate, pState->m_loFreq, pState->m_hiFreq);
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
        pState->m_delay2->process(pInput, pState->m_pBandBuf, numSamples);
        pState->m_low2->process(pState->m_pBandBuf, pState->m_pBandBuf, numSamples);
    } else {
        pState->m_delay2->pauseFilter();
        pState->m_low2->pauseFilter();
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

    if (enableState == EffectProcessor::DISABLING) {
        pState->m_delay3->pauseFilter();
        pState->m_low2->pauseFilter();
        pState->m_low1->pauseFilter();
    }
}
