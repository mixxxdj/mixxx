#include "effects/native/biquadstaticeqeffect.h"
#include "util/math.h"

// static
QString BiquadStaticEQEffect::getId() {
    return "org.mixxx.effects.biquadstaticeq";
}

// static
EffectManifest BiquadStaticEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Biquad Static EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Biquad 10 band static EQ"));

    for (int i = 0; i < 10; i++) {
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

    return manifest;
}

BiquadStaticEQEffectGroupState::BiquadStaticEQEffectGroupState() {
    for (int i = 0; i < 10; i++) {
        m_pBandBuf.append(SampleUtil::alloc(MAX_BUFFER_LEN));
        old_mid.append(1.0);
    }

    // Initialize filters with the default values
    // TODO(rryan): use the real samplerate
    band.append(new EngineFilterBiquad1Band(44100, 31.25));
    band.append(new EngineFilterBiquad1Band(44100, 62.5));
    band.append(new EngineFilterBiquad1Band(44100, 125.0));
    band.append(new EngineFilterBiquad1Band(44100, 250.0));
    band.append(new EngineFilterBiquad1Band(44100, 500.0));
    band.append(new EngineFilterBiquad1Band(44100, 1000.0));
    band.append(new EngineFilterBiquad1Band(44100, 2000.0));
    band.append(new EngineFilterBiquad1Band(44100, 4000.0));
    band.append(new EngineFilterBiquad1Band(44100, 8000.0));
    band.append(new EngineFilterBiquad1Band(44100, 16000.0));

}

BiquadStaticEQEffectGroupState::~BiquadStaticEQEffectGroupState() {
    foreach (EngineFilterBiquad1Band* filter, band) {
        delete filter;
    }
    foreach (CSAMPLE* buf, m_pBandBuf) {
        SampleUtil::free(buf);
    }
}

BiquadStaticEQEffect::BiquadStaticEQEffect(EngineEffect* pEffect,
                                         const EffectManifest& manifest) {
    Q_UNUSED(manifest);
    for (int i = 0; i < 10; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
}

BiquadStaticEQEffect::~BiquadStaticEQEffect() {
}

void BiquadStaticEQEffect::processGroup(const QString& group,
                                       BiquadStaticEQEffectGroupState* pState,
                                       const CSAMPLE* pInput, CSAMPLE* pOutput,
                                       const unsigned int numSamples,
                                       const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fMid[10] = { };
    for (int i = 0; i < 10; i++) {
        fMid[i] = m_pPotMid[i]->value().toDouble();
    }

    // Process the new EQ'd signals.
    // They use up to 16 frames history so in case we are just starting,
    // 16 frames are junk, this is handled by ramp_delay
    int ramp_delay = 0;

    for (int i = 0; i < 10; i++) {
        if (fMid[i] || pState->old_mid[i]) {
            pState->band[i]->process(pInput, pState->m_pBandBuf[i], numSamples);
            if(pState->old_mid[i] == 0) {
                ramp_delay = 30;
            }
        }
    }

    if (ramp_delay) {
        // first use old gains
        SampleUtil::copy10WithGain(pOutput,
                pState->m_pBandBuf[0], pState->old_mid[0],
                pState->m_pBandBuf[1], pState->old_mid[1],
                pState->m_pBandBuf[2], pState->old_mid[2],
                pState->m_pBandBuf[3], pState->old_mid[3],
                pState->m_pBandBuf[4], pState->old_mid[4],
                pState->m_pBandBuf[5], pState->old_mid[5],
                pState->m_pBandBuf[6], pState->old_mid[6],
                pState->m_pBandBuf[7], pState->old_mid[7],
                pState->m_pBandBuf[8], pState->old_mid[8],
                pState->m_pBandBuf[9], pState->old_mid[9],
                ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy10WithRampingGain(&pOutput[ramp_delay],
                &pState->m_pBandBuf[0][ramp_delay], pState->old_mid[0], fMid[0],
                &pState->m_pBandBuf[1][ramp_delay], pState->old_mid[1], fMid[1],
                &pState->m_pBandBuf[2][ramp_delay], pState->old_mid[2], fMid[2],
                &pState->m_pBandBuf[3][ramp_delay], pState->old_mid[3], fMid[3],
                &pState->m_pBandBuf[4][ramp_delay], pState->old_mid[4], fMid[4],
                &pState->m_pBandBuf[5][ramp_delay], pState->old_mid[5], fMid[5],
                &pState->m_pBandBuf[6][ramp_delay], pState->old_mid[6], fMid[6],
                &pState->m_pBandBuf[7][ramp_delay], pState->old_mid[7], fMid[7],
                &pState->m_pBandBuf[8][ramp_delay], pState->old_mid[8], fMid[8],
                &pState->m_pBandBuf[9][ramp_delay], pState->old_mid[9], fMid[9],
                numSamples - ramp_delay);
    } else if (fMid[0] != pState->old_mid[0] ||
               fMid[1] != pState->old_mid[1] ||
               fMid[2] != pState->old_mid[2] ||
               fMid[3] != pState->old_mid[3] ||
               fMid[4] != pState->old_mid[4] ||
               fMid[5] != pState->old_mid[5] ||
               fMid[6] != pState->old_mid[6] ||
               fMid[7] != pState->old_mid[7] ||
               fMid[8] != pState->old_mid[8] ||
               fMid[9] != pState->old_mid[9]) {
        SampleUtil::copy10WithRampingGain(pOutput,
                pState->m_pBandBuf[0], pState->old_mid[0], fMid[0],
                pState->m_pBandBuf[1], pState->old_mid[1], fMid[1],
                pState->m_pBandBuf[2], pState->old_mid[2], fMid[2],
                pState->m_pBandBuf[3], pState->old_mid[3], fMid[3],
                pState->m_pBandBuf[4], pState->old_mid[4], fMid[4],
                pState->m_pBandBuf[5], pState->old_mid[5], fMid[5],
                pState->m_pBandBuf[6], pState->old_mid[6], fMid[6],
                pState->m_pBandBuf[7], pState->old_mid[7], fMid[7],
                pState->m_pBandBuf[8], pState->old_mid[8], fMid[8],
                pState->m_pBandBuf[9], pState->old_mid[9], fMid[9],
                numSamples);
    } else {
        SampleUtil::copy10WithGain(pOutput,
                pState->m_pBandBuf[0], fMid[0],
                pState->m_pBandBuf[1], fMid[1],
                pState->m_pBandBuf[2], fMid[2],
                pState->m_pBandBuf[3], fMid[3],
                pState->m_pBandBuf[4], fMid[4],
                pState->m_pBandBuf[5], fMid[5],
                pState->m_pBandBuf[6], fMid[6],
                pState->m_pBandBuf[7], fMid[7],
                pState->m_pBandBuf[8], fMid[8],
                pState->m_pBandBuf[9], fMid[9],
                numSamples);
    }

    for (int i = 0; i < 10; i++) {
        pState->old_mid[i] = fMid[i];
    }
}
