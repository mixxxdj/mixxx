#include "effects/backends/builtin/bessel4lvmixeqeffect.h"

#include "effects/backends/builtin/equalizer_util.h"
#include "util/math.h"

// static
QString Bessel4LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel4lvmixeq";
}

// static
EffectManifestPointer Bessel4LVMixEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Bessel4 LV-Mix Isolator"));
    pManifest->setShortName(QObject::tr("Bessel4 ISO"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A Bessel 4th-order filter isolator with Lipshitz and "
                        "Vanderkooy mix (bit perfect unity, roll-off -24 "
                        "dB/octave).") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setIsMixingEQ(true);
    pManifest->setEffectRampsFromDry(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), false);
    return pManifest;
}

Bessel4LVMixEQEffect::Bessel4LVMixEQEffect() {
    m_pLoFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "HiEQFrequency");
    m_pEQButtonMode = std::make_unique<ControlProxy>("[Mixer Profile]", "EQButtonMode");
}

void Bessel4LVMixEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotMid = parameters.value("mid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillMid = parameters.value("killMid");
    m_pKillHigh = parameters.value("killHigh");
}

void Bessel4LVMixEQEffect::processChannel(
        Bessel4LVMixEQEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        pState->processChannelAndPause(pInput, pOutput, engineParameters.samplesPerBuffer());
    } else {
        double fLow;
        double fMid;
        double fHigh;
        if (!m_pKillLow->toBool()) {
            fLow = m_pPotLow->value();
        } else {
            fLow = m_pEQButtonMode->get() == 0 ? 0 : 1;
        }
        if (!m_pKillMid->toBool()) {
            fMid = m_pPotMid->value();
        } else {
            fMid = m_pEQButtonMode->get() == 0 ? 0 : 1;
        }
        if (!m_pKillHigh->toBool()) {
            fHigh = m_pPotHigh->value();
        } else {
            fHigh = m_pEQButtonMode->get() == 0 ? 0 : 1;
        }
        pState->processChannel(pInput,
                pOutput,
                engineParameters.samplesPerBuffer(),
                engineParameters.sampleRate(),
                fLow,
                fMid,
                fHigh,
                m_pLoFreqCorner->get(),
                m_pHiFreqCorner->get());
    }
}
