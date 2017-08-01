#include "effects/native/semiparametriceq4knobeffect.h"

namespace {
static const int kStartupSamplerate = 44100;
static const double kMinCorner = 13; // Hz
static const double kMaxCorner = 22050; // Hz
static const double kLpfHpfQ = 0.707106781;
static const double kSemiparametricQ = 0.4;
static const double kSemiparametricMaxBoostDb = 8;
static const double kSemiparametricMaxCutDb = -20;
} // anonymous namespace

SemiparametricEQEffect4KnobGroupState::SemiparametricEQEffect4KnobGroupState()
        : m_lowFilter(1, kMaxCorner / kStartupSamplerate, kLpfHpfQ, true),
          m_semiParametricFilter(kStartupSamplerate, 1000, kSemiparametricQ),
          m_highFilter(1, kMinCorner / kStartupSamplerate, kLpfHpfQ, true),
          m_intermediateBuffer(MAX_BUFFER_LEN),
          m_dLpfOld(0),
          m_dCenterOld(0),
          m_dGainOld(0),
          m_dHpfOld(0) {
}

// static
QString SemiparametricEQEffect4Knob::getId() {
    return "org.mixxx.effects.semiparametriceq4knob";
}

EffectManifest SemiparametricEQEffect4Knob::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Semiparametric Equalizer (4 knobs)"));
    manifest.setShortName(QObject::tr("Semiparam 4"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A semiparametric EQ effect modeled after the PLAYdifferently Model 1 hardware mixer."));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMixingEQ(true);

    EffectManifestParameter* hpf = manifest.addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    hpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(kMinCorner);
    hpf->setMinimum(kMinCorner);
    hpf->setMaximum(kMaxCorner);

    EffectManifestParameter* gain = manifest.addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Gain of the semiparametric EQ"));
    gain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    gain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::DECIBELS);
    gain->setMinimum(-20);
    gain->setMaximum(8);
    gain->setDefault(0);

    EffectManifestParameter* center = manifest.addParameter();
    center->setId("center");
    center->setName(QObject::tr("Center"));
    center->setDescription(QObject::tr("Center frequency of the semiparametric EQ"));
    center->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    center->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    center->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    center->setMinimum(70);
    center->setMaximum(7000);
    center->setDefault(1000);

    EffectManifestParameter* lpf = manifest.addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lpf->setNeutralPointOnScale(1);
    lpf->setMinimum(kMinCorner);
    lpf->setMaximum(kMaxCorner);
    lpf->setDefault(kMaxCorner);

    return manifest;
}

SemiparametricEQEffect4Knob::SemiparametricEQEffect4Knob(EngineEffect* pEffect,
                                               const EffectManifest& manifest)
        : m_pLPF(pEffect->getParameterById("lpf")),
          m_pCenter(pEffect->getParameterById("center")),
          m_pGain(pEffect->getParameterById("gain")),
          m_pHPF(pEffect->getParameterById("hpf")) {
    Q_UNUSED(manifest);
}

void SemiparametricEQEffect4Knob::processChannel(const ChannelHandle& handle,
                                                 SemiparametricEQEffect4KnobGroupState* pState,
                                                 const CSAMPLE* pInput, CSAMPLE *pOutput,
                                                 const unsigned int numSamples,
                                                 const unsigned int sampleRate,
                                                 const EffectProcessor::EnableState enableState,
                                                 const GroupFeatureState& groupFeatureState) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatureState);
    Q_UNUSED(enableState);

    double lpf = m_pLPF->value() / sampleRate;
    double center = m_pCenter->value();
    double gain = m_pGain->value();
    double hpf = m_pHPF->value() / sampleRate;

    if (lpf != pState->m_dLpfOld) {
        pState->m_lowFilter.setFrequencyCorners(1, lpf, kLpfHpfQ);
    }
    if (center != pState->m_dCenterOld || gain != pState->m_dGainOld) {
        double db = gain - 1.0;
        if (db >= 0) {
            db *= kSemiparametricMaxBoostDb;
        } else {
            db *= -kSemiparametricMaxCutDb;
        }
        pState->m_semiParametricFilter.setFrequencyCorners(sampleRate, center, kSemiparametricQ, db);
    }
    if (hpf != pState->m_dHpfOld) {
        pState->m_highFilter.setFrequencyCorners(1, hpf, kLpfHpfQ);
    }

    pState->m_lowFilter.process(pInput, pState->m_intermediateBuffer.data(), numSamples);
    pState->m_semiParametricFilter.process(pState->m_intermediateBuffer.data(), pState->m_intermediateBuffer.data(), numSamples);
    pState->m_highFilter.process(pState->m_intermediateBuffer.data(), pOutput, numSamples);

    pState->m_dLpfOld = lpf;
    pState->m_dCenterOld = center;
    pState->m_dGainOld = gain;
    pState->m_dHpfOld = hpf;
}
