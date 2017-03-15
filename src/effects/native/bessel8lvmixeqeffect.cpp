#include "effects/native/bessel8lvmixeqeffect.h"

#include "effects/native/equalizer_util.h"
#include "util/math.h"

// static
QString Bessel8LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel8lvmixeq";
}

// static
EffectManifest Bessel8LVMixEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Bessel8 LV-Mix Isolator"));
    manifest.setShortName(QObject::tr("Bessel8 ISO"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Bessel 8th-order filter isolator with Lipshitz and Vanderkooy mix (bit perfect unity, roll-off -48 dB/octave).") + " " + EqualizerUtil::adjustFrequencyShelvesTip());
    manifest.setIsMixingEQ(true);
    manifest.setEffectRampsFromDry(true);

    EqualizerUtil::createCommonParameters(&manifest);
    return manifest;
}

Bessel8LVMixEQEffect::Bessel8LVMixEQEffect(EngineEffect* pEffect,
                                           const EffectManifest& manifest)
        : m_pPotLow(pEffect->getParameterById("low")),
          m_pPotMid(pEffect->getParameterById("mid")),
          m_pPotHigh(pEffect->getParameterById("high")),
          m_pKillLow(pEffect->getParameterById("killLow")),
          m_pKillMid(pEffect->getParameterById("killMid")),
          m_pKillHigh(pEffect->getParameterById("killHigh")) {
    Q_UNUSED(manifest);
    m_pLoFreqCorner = new ControlProxy("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlProxy("[Mixer Profile]", "HiEQFrequency");
}

Bessel8LVMixEQEffect::~Bessel8LVMixEQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void Bessel8LVMixEQEffect::processChannel(const ChannelHandle& handle,
                                          Bessel8LVMixEQEffectGroupState* pState,
                                          const CSAMPLE* pInput, CSAMPLE* pOutput,
                                          const unsigned int numSamples,
                                          const unsigned int sampleRate,
                                          const EffectProcessor::EnableState enableState,
                                          const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);


    if (enableState == EffectProcessor::DISABLING) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        pState->processChannelAndPause(pInput, pOutput, numSamples);
    } else {
        double fLow;
        double fMid;
        double fHigh;
        if (!m_pKillLow->toBool()) {
            fLow = m_pPotLow->value();
        } else {
            fLow = 0;
        }
        if (!m_pKillMid->toBool()) {
            fMid = m_pPotMid->value();
        } else {
            fMid = 0;
        }
        if (!m_pKillHigh->toBool()) {
            fHigh = m_pPotHigh->value();
        } else {
            fHigh = 0;
        }
        pState->processChannel(
                pInput, pOutput, numSamples, sampleRate, fLow, fMid, fHigh,
                m_pLoFreqCorner->get(), m_pHiFreqCorner->get());
    }
}
