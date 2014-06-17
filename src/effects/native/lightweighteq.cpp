#include "effects/native/lightweighteq.h"
#include "util/math.h"

// static
QString LightweightEQ::getId() {
    return "org.mixxx.effects.lightweighteq";
}

// static
EffectManifest LightweightEQ::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Lightweight EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("Lightweight Static Equalizer which can be used on \
machines with lower CPU performances");

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("lightweight_low");
    low->setName(QObject::tr("Low"));
    low->setDescription("Gain for Low Filter");
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setDefault(1.);
    low->setMinimum(0);
    low->setMaximum(4.);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("lightweight_mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription("Gain for Band Filter");
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setDefault(1.);
    mid->setMinimum(0);
    mid->setMaximum(4.);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("lightweight_high");
    high->setName(QObject::tr("High"));
    high->setDescription("Gain for High Filter");
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.);
    high->setMinimum(0);
    high->setMaximum(4.);

    return manifest;
}

LightweightEQGroupState::LightweightEQGroupState()
        : low(NULL), band(NULL), high(NULL),old_low(1.0),
          old_mid(1.0), old_high(1.0), old_dry(0) {
    m_pLowBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pBandBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pHighBuf = new CSAMPLE[MAX_BUFFER_LEN];

    // Initialize filters with the default values
    low = new EngineFilterIIR(bessel_lowpass4, 4);
    band = new EngineFilterIIR(bessel_bandpass, 8);
    high = new EngineFilterIIR(bessel_highpass4, 4);
}

LightweightEQGroupState::~LightweightEQGroupState() {
    delete low;
    delete band;
    delete high;
    delete m_pLowBuf;
    delete m_pBandBuf;
    delete m_pHighBuf;
}

LightweightEQ::LightweightEQ(EngineEffect* pEffect,
                   const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("lightweight_low")),
          m_pPotMid(pEffect->getParameterById("lightweight_mid")),
          m_pPotHigh(pEffect->getParameterById("lightweight_high")) {
    Q_UNUSED(manifest);
}

LightweightEQ::~LightweightEQ() {
    //qDebug() << debugString() << "destroyed";
}

void LightweightEQ::processGroup(const QString& group,
        LightweightEQGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    fLow = m_pPotLow->value().toDouble();
    fMid = m_pPotMid->value().toDouble();
    fHigh = m_pPotHigh->value().toDouble();

    float fDry = 0;
    // This is the RGBW Mix. It is currently not working,
    // because of the group delay, introduced by the filters.
    // Since the dry signal has no delay, we get a frequency distorsion
    // once it is mixed together with the filtered signal
    // This might be fixed later by an allpass filter for the dry signal
    // or zero-phase no-lag filters
    // "Linear Phase EQ" "filtfilt()"
    //fDry = qMin(qMin(fLow, fMid), fHigh);
    //fLow -= fDry;
    //fMid -= fDry;
    //fHigh -= fDry;

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
    if (fMid || pState->old_mid) {
        pState->band->process(pInput, pState->m_pBandBuf, numSamples);
        if(pState->old_mid== 0) {
            ramp_delay = 30;
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
        SampleUtil::copy4WithGain(pOutput,
                pInput, pState->old_dry,
                pState->m_pLowBuf, pState->old_low,
                pState->m_pBandBuf, pState->old_mid,
                pState->m_pHighBuf, pState->old_high,
                ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy4WithRampingGain(&pOutput[ramp_delay],
                &pInput[ramp_delay], pState->old_dry, fDry,
                &pState->m_pLowBuf[ramp_delay], pState->old_low, fLow,
                &pState->m_pBandBuf[ramp_delay], pState->old_mid, fMid,
                &pState->m_pHighBuf[ramp_delay], pState->old_high, fHigh,
                numSamples - ramp_delay);
    } else if (fLow != pState->old_low ||
            fMid != pState->old_mid ||
            fHigh != pState->old_high ||
            fDry != pState->old_dry) {
        SampleUtil::copy4WithRampingGain(pOutput,
                pInput, pState->old_dry, fDry,
                pState->m_pLowBuf, pState->old_low, fLow,
                pState->m_pBandBuf, pState->old_mid, fMid,
                pState->m_pHighBuf, pState->old_high, fHigh,
                numSamples);
    } else {
        SampleUtil::copy4WithGain(pOutput,
                pInput, fDry,
                pState->m_pLowBuf, fLow,
                pState->m_pBandBuf, fMid,
                pState->m_pHighBuf, fHigh,
                numSamples);
    }

    pState->old_low = fLow;
    pState->old_mid = fMid;
    pState->old_high = fHigh;
    pState->old_dry = fDry;
}
