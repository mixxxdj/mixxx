#include "effects/backends/builtin/bessel8lvmixeqeffect.h"

#include "effects/backends/builtin/equalizer_util.h"
#include "util/math.h"

// static
QString Bessel8LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel8lvmixeq";
}

// static
EffectManifestPointer Bessel8LVMixEQEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Bessel8 LV-Mix Isolator"));
    pManifest->setShortName(QObject::tr("Bessel8 ISO"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("A Bessel 8th-order filter isolator with Lipshitz and "
                        "Vanderkooy mix (bit perfect unity, roll-off -48 "
                        "dB/octave).") +
            " " + EqualizerUtil::adjustFrequencyShelvesTip());
    pManifest->setIsMixingEQ(true);
    pManifest->setEffectRampsFromDry(true);

    EqualizerUtil::createCommonParameters(pManifest.data(), false);
    return pManifest;
}

Bessel8LVMixEQEffect::Bessel8LVMixEQEffect() {
    m_pLoFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = std::make_unique<ControlProxy>("[Mixer Profile]", "HiEQFrequency");
}

void Bessel8LVMixEQEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pPotLow = parameters.value("low");
    m_pPotMid = parameters.value("mid");
    m_pPotHigh = parameters.value("high");
    m_pKillLow = parameters.value("killLow");
    m_pKillMid = parameters.value("killMid");
    m_pKillHigh = parameters.value("killHigh");
    m_pBypassLow = parameters.value("bypassLow");
    m_pBypassMid = parameters.value("bypassMid");
    m_pBypassHigh = parameters.value("bypassHigh");
}

void Bessel8LVMixEQEffect::processChannel(
        Bessel8LVMixEQEffectGroupState* pState,
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
        if (m_pBypassLow->toBool()) {
            fLow = 1;
        } else if (m_pKillLow->toBool()) {
            fLow = 0;
        } else {
            fLow = m_pPotLow->value();
        }
        if (m_pBypassMid->toBool()) {
            fMid = 1;
        } else if (m_pKillMid->toBool()) {
            fMid = 0;
        } else {
            fMid = m_pPotMid->value();
        }
        if (m_pBypassHigh->toBool()) {
            fHigh = 1;
        } else if (m_pKillHigh->toBool()) {
            fHigh = 0;
        } else {
            fHigh = m_pPotHigh->value();
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
