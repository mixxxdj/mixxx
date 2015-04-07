#include "effects/native/bessel8lvmixeqeffect.h"
#include "util/math.h"

// static
QString Bessel8LVMixEQEffect::getId() {
    return "org.mixxx.effects.bessel8lvmixeq";
}

// static
EffectManifest Bessel8LVMixEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Bessel8 LV-Mix EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A Bessel 8th order filter equalizer with Lipshitz and Vanderkooy mix (bit perfect unity, roll-off -48 db/Oct). "
        "To adjust frequency shelves see the Equalizer preferences."));
    manifest.setIsMixingEQ(true);
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

    EffectManifestParameter* killLow = manifest.addParameter();
    killLow->setId("killLow");
    killLow->setName(QObject::tr("Kill Low"));
    killLow->setDescription(QObject::tr("Kill the Low Filter"));
    killLow->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killLow->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killLow->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killLow->setDefault(0);
    killLow->setMinimum(0);
    killLow->setMaximum(1);

    EffectManifestParameter* mid = manifest.addParameter();
    mid->setId("mid");
    mid->setName(QObject::tr("Mid"));
    mid->setDescription(QObject::tr("Gain for Band Filter"));
    mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    mid->setNeutralPointOnScale(0.5);
    mid->setDefault(1.0);
    mid->setMinimum(0);
    mid->setMaximum(4.0);

    EffectManifestParameter* killMid = manifest.addParameter();
    killMid->setId("killMid");
    killMid->setName(QObject::tr("Kill Mid"));
    killMid->setDescription(QObject::tr("Kill the Mid Filter"));
    killMid->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killMid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killMid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killMid->setDefault(0);
    killMid->setMinimum(0);
    killMid->setMaximum(1);

    EffectManifestParameter* high = manifest.addParameter();
    high->setId("high");
    high->setName(QObject::tr("High"));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setNeutralPointOnScale(0.5);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    EffectManifestParameter* killHigh = manifest.addParameter();
    killHigh->setId("killHigh");
    killHigh->setName(QObject::tr("Kill High"));
    killHigh->setDescription(QObject::tr("Kill the High Filter"));
    killHigh->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killHigh->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killHigh->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killHigh->setDefault(0);
    killHigh->setMinimum(0);
    killHigh->setMaximum(1);

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
    m_pLoFreqCorner = new ControlObjectSlave("[Mixer Profile]", "LoEQFrequency");
    m_pHiFreqCorner = new ControlObjectSlave("[Mixer Profile]", "HiEQFrequency");
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

    double fLow;
    double fMid;
    double fHigh;
    if (enableState == EffectProcessor::DISABLING) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        fLow = 1.0;
        fMid = 1.0;
        fHigh = 1.0;
    } else {
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
    }

    pState->processChannel(pInput, pOutput, numSamples, sampleRate,
                           fLow, fMid, fHigh,
                           m_pLoFreqCorner->get(), m_pHiFreqCorner->get());
}
