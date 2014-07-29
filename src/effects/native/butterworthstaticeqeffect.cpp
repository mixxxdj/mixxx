#include "effects/native/butterworthstaticeqeffect.h"
#include "util/math.h"

// static
QString ButterworthStaticEQEffect::getId() {
    return "org.mixxx.effects.butterworthstaticeq";
}

// static
EffectManifest ButterworthStaticEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Butterworth Static EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Butterworth 10 band static EQ"));

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

    for (int i = 0; i < 9; i++) {
        EffectManifestParameter* mid = manifest.addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(QString("Mid%1").arg(i));
        mid->setDescription(QString("Gain for Band Filter %1").arg(i));
        mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
        mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
        mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        mid->setDefault(1.0);
        mid->setMinimum(0);
        mid->setMaximum(4.0);
    }

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

ButterworthStaticEQEffectGroupState::ButterworthStaticEQEffectGroupState()
        : low(NULL), high(NULL), old_low(1.0),
          old_high(1.0) {
    m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    for (int i = 0; i < 9; i++) {
        m_pBandBuf.append(SampleUtil::alloc(MAX_BUFFER_LEN));
        old_mid.append(1.0);
    }
    m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize filters with the default values
    // TODO(rryan): use the real samplerate
    low = new EngineFilterButterworth8Low(44100, 31.25);
    band.append(new EngineFilterButterworth8Band(44100, 31.25, 62.5));
    band.append(new EngineFilterButterworth8Band(44100, 62.5, 125.0));
    band.append(new EngineFilterButterworth8Band(44100, 125.0, 250.0));
    band.append(new EngineFilterButterworth8Band(44100, 250.0, 500.0));
    band.append(new EngineFilterButterworth8Band(44100, 500.0, 1000.0));
    band.append(new EngineFilterButterworth8Band(44100, 1000.0, 2000.0));
    band.append(new EngineFilterButterworth8Band(44100, 2000.0, 4000.0));
    band.append(new EngineFilterButterworth8Band(44100, 4000.0, 8000.0));
    band.append(new EngineFilterButterworth8Band(44100, 8000.0, 16000.0));
    high = new EngineFilterButterworth8High(44100, 16000.0);

}

ButterworthStaticEQEffectGroupState::~ButterworthStaticEQEffectGroupState() {
    delete low;
    foreach (EngineFilterButterworth8Band* filter, band) {
        delete filter;
    }
    delete high;
    SampleUtil::free(m_pLowBuf);
    foreach (CSAMPLE* buf, m_pBandBuf) {
        SampleUtil::free(buf);
    }
    SampleUtil::free(m_pHighBuf);
}

ButterworthStaticEQEffect::ButterworthStaticEQEffect(EngineEffect* pEffect,
                                         const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotHigh(pEffect->getParameterById("high")) {
    Q_UNUSED(manifest);
    for (int i = 0; i < 9; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
}

ButterworthStaticEQEffect::~ButterworthStaticEQEffect() {
}

void ButterworthStaticEQEffect::processGroup(const QString& group,
                                       ButterworthStaticEQEffectGroupState* pState,
                                       const CSAMPLE* pInput, CSAMPLE* pOutput,
                                       const unsigned int numSamples,
                                       const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid[9] = { }, fHigh = 0.f;
    fLow = m_pPotLow->value().toDouble();
    for (int i = 0; i < 9; i++) {
        fMid[i] = m_pPotMid[i]->value().toDouble();
    }
    fHigh = m_pPotHigh->value().toDouble();

    // Process the new EQ'd signals.
    // They use up to 16 frames history so in case we are just starting,
    // 16 frames are junk, this is handled by ramp_delay
    int ramp_delay = 0;
    if (fLow || pState->old_low) {
        pState->low->process(pInput, pState->m_pLowBuf, numSamples);
        if(pState->old_low == 0) {
            ramp_delay = 30;
        }
    }

    for (int i = 0; i < 9; i++) {
        if (fMid[i] || pState->old_mid[i]) {
            pState->band[i]->process(pInput, pState->m_pBandBuf[i], numSamples);
            if(pState->old_mid[i] == 0) {
                ramp_delay = 30;
            }
        }
    }

    if (fHigh || pState->old_high) {
        pState->high->process(pInput, pState->m_pHighBuf, numSamples);
        if(pState->old_high == 0) {
            ramp_delay = 30;
        }
    }

    if (ramp_delay) {
        // first use old gains
        SampleUtil::copy11WithGain(pOutput,
                pState->m_pLowBuf, pState->old_low,
                pState->m_pBandBuf[0], pState->old_mid[0],
                pState->m_pBandBuf[1], pState->old_mid[1],
                pState->m_pBandBuf[2], pState->old_mid[2],
                pState->m_pBandBuf[3], pState->old_mid[3],
                pState->m_pBandBuf[4], pState->old_mid[4],
                pState->m_pBandBuf[5], pState->old_mid[5],
                pState->m_pBandBuf[6], pState->old_mid[6],
                pState->m_pBandBuf[7], pState->old_mid[7],
                pState->m_pBandBuf[8], pState->old_mid[8],
                pState->m_pHighBuf, pState->old_high,
                ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy11WithRampingGain(&pOutput[ramp_delay],
                &pState->m_pLowBuf[ramp_delay], pState->old_low, fLow,
                &pState->m_pBandBuf[0][ramp_delay], pState->old_mid[0], fMid[0],
                &pState->m_pBandBuf[1][ramp_delay], pState->old_mid[1], fMid[1],
                &pState->m_pBandBuf[2][ramp_delay], pState->old_mid[2], fMid[2],
                &pState->m_pBandBuf[3][ramp_delay], pState->old_mid[3], fMid[3],
                &pState->m_pBandBuf[4][ramp_delay], pState->old_mid[4], fMid[4],
                &pState->m_pBandBuf[5][ramp_delay], pState->old_mid[5], fMid[5],
                &pState->m_pBandBuf[6][ramp_delay], pState->old_mid[6], fMid[6],
                &pState->m_pBandBuf[7][ramp_delay], pState->old_mid[7], fMid[7],
                &pState->m_pBandBuf[8][ramp_delay], pState->old_mid[8], fMid[8],
                &pState->m_pHighBuf[ramp_delay], pState->old_high, fHigh,
                numSamples - ramp_delay);
    } else if (fLow != pState->old_low ||
            fMid[0] != pState->old_mid[0] ||
            fMid[1] != pState->old_mid[1] ||
            fMid[2] != pState->old_mid[2] ||
            fMid[3] != pState->old_mid[3] ||
            fMid[4] != pState->old_mid[4] ||
            fMid[5] != pState->old_mid[5] ||
            fMid[6] != pState->old_mid[6] ||
            fMid[7] != pState->old_mid[7] ||
            fMid[8] != pState->old_mid[8] ||
            fHigh != pState->old_high) {
        SampleUtil::copy11WithRampingGain(pOutput,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf[0], pState->old_mid[0], fMid[0],
                pState->m_pBandBuf[1], pState->old_mid[1], fMid[1],
                pState->m_pBandBuf[2], pState->old_mid[2], fMid[2],
                pState->m_pBandBuf[3], pState->old_mid[3], fMid[3],
                pState->m_pBandBuf[4], pState->old_mid[4], fMid[4],
                pState->m_pBandBuf[5], pState->old_mid[5], fMid[5],
                pState->m_pBandBuf[6], pState->old_mid[6], fMid[6],
                pState->m_pBandBuf[7], pState->old_mid[7], fMid[7],
                pState->m_pBandBuf[8], pState->old_mid[8], fMid[8],
                pState->m_pHighBuf, pState->old_high, fHigh,
                numSamples);
    } else {
        SampleUtil::copy11WithGain(pOutput,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf[0], fMid[0],
                pState->m_pBandBuf[1], fMid[1],
                pState->m_pBandBuf[2], fMid[2],
                pState->m_pBandBuf[3], fMid[3],
                pState->m_pBandBuf[4], fMid[4],
                pState->m_pBandBuf[5], fMid[5],
                pState->m_pBandBuf[6], fMid[6],
                pState->m_pBandBuf[7], fMid[7],
                pState->m_pBandBuf[8], fMid[8],
                pState->m_pHighBuf, fHigh,
                numSamples);
    }

    pState->old_low = fLow;
    for (int i = 0; i < 9; i++) {
        pState->old_mid[i] = fMid[i];
    }
    pState->old_high = fHigh;
}
