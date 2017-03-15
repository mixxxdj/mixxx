#include "effects/native/bessel4lvmixeqeffect.h"

#include "effects/native/equalizer_util.h"
#include "util/math.h"

// static
QString Bessel4LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel4lvmixeq";
}

// static
EffectManifest Bessel4LVMixEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Bessel4 LV-Mix Isolator"));
    manifest.setShortName(QObject::tr("Bessel4 ISO"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Bessel 4th-order filter isolator with Lipshitz and Vanderkooy mix (bit perfect unity, roll-off -24 dB/octave).") + " " + EqualizerUtil::adjustFrequencyShelvesTip());
    manifest.setIsMixingEQ(true);
    manifest.setEffectRampsFromDry(true);

    EqualizerUtil::createCommonParameters(&manifest);
    return manifest;
}

Bessel4LVMixEQEffect::Bessel4LVMixEQEffect(EngineEffect* pEffect,
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

Bessel4LVMixEQEffect::~Bessel4LVMixEQEffect() {
    delete m_pLoFreqCorner;
    delete m_pHiFreqCorner;
}

void Bessel4LVMixEQEffect::processChannel(const ChannelHandle& handle,
                                          Bessel4LVMixEQEffectGroupState* pState,
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
        pState->processChannel(pInput, pOutput, numSamples, sampleRate,
                               fLow, fMid, fHigh,
                               m_pLoFreqCorner->get(), m_pHiFreqCorner->get());
    }
}
