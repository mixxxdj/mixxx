#include "effects/native/semiparametriceq3knobeffect.h"

namespace {
static const int kStartupSamplerate = 44100;
static const double kMinCorner = 13; // Hz
static const double kMaxCorner = 22050; // Hz
static const double kLpfHpfQ = 0.1;
static const double kSemiparametricQ = 0.4;
static const double kSemiparametricMaxBoostDb = 8;
static const double kSemiparametricMaxCutDb = -20;
} // anonymous namespace

SemiparametricEQEffect3KnobGroupState::SemiparametricEQEffect3KnobGroupState()
        : m_lowFilter(1, kMaxCorner / kStartupSamplerate, kLpfHpfQ, true),
          m_semiParametricFilter(kStartupSamplerate, 1000, kSemiparametricQ),
          m_highFilter(1, kMinCorner / kStartupSamplerate, kLpfHpfQ, true),
          m_intermediateBuffer(MAX_BUFFER_LEN),
          m_filterBehavior(kMinCorner, kMaxCorner, -40),
          m_dCenterOld(0),
          m_dGainOld(0),
          m_dFilterOld(0) {
}

// static
QString SemiparametricEQEffect3Knob::getId() {
    return "org.mixxx.effects.semiparametriceq3knob";
}

EffectManifest SemiparametricEQEffect3Knob::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Semiparametric Equalizer (3 knobs)"));
    manifest.setShortName(QObject::tr("Semiparam 3"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "A semiparametric EQ effect modeled after the PLAYdifferently Model 1 hardware mixer."));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMixingEQ(true);

    EffectManifestParameter* filter = manifest.addParameter();
    filter->setId("filter");
    filter->setName(QObject::tr("Filter"));
    filter->setDescription(QObject::tr("Bipolar filter knob. Controls corner frequency ratio of the high pass filter on the left and corner frequency of the low pass filter on the right."));
    filter->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    filter->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    filter->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    filter->setMinimum(0);
    filter->setMaximum(1);
    filter->setDefault(0.5);

    EffectManifestParameter* gain = manifest.addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Gain of the semiparametric EQ"));
    gain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    gain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::DECIBELS);
    gain->setMinimum(0);
    gain->setMaximum(4);
    gain->setDefault(1);

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

    return manifest;
}

SemiparametricEQEffect3Knob::SemiparametricEQEffect3Knob(EngineEffect* pEffect,
                                               const EffectManifest& manifest)
        : m_pCenter(pEffect->getParameterById("center")),
          m_pGain(pEffect->getParameterById("gain")),
          m_pFilter(pEffect->getParameterById("filter")) {
    Q_UNUSED(manifest);
}

void SemiparametricEQEffect3Knob::processChannel(const ChannelHandle& handle,
                                                 SemiparametricEQEffect3KnobGroupState* pState,
                                                 const CSAMPLE* pInput, CSAMPLE *pOutput,
                                                 const unsigned int numSamples,
                                                 const unsigned int sampleRate,
                                                 const EffectProcessor::EnableState enableState,
                                                 const GroupFeatureState& groupFeatureState) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatureState);
    Q_UNUSED(enableState);

    double center = m_pCenter->value();
    double gain = m_pGain->value();
    double filter = m_pFilter->value();

    if (center != pState->m_dCenterOld || gain != pState->m_dGainOld) {
        double db = gain - 1.0;
        if (db >= 0) {
            db *= kSemiparametricMaxBoostDb;
        } else {
            db *= -kSemiparametricMaxCutDb;
        }
        pState->m_semiParametricFilter.setFrequencyCorners(sampleRate, center, kSemiparametricQ, db);
    }
    if (filter != pState->m_dFilterOld) {
        double lpf = pState->m_filterBehavior.parameterToValue(
                std::min((filter * 2.0), 1.0)) / sampleRate;
        double hpf = pState->m_filterBehavior.parameterToValue(
                std::max((filter - 0.5) * 2.0, 0.0)) / sampleRate;
        pState->m_lowFilter.setFrequencyCorners(1, lpf, kLpfHpfQ);
        pState->m_highFilter.setFrequencyCorners(1, hpf, kLpfHpfQ);
    }

    pState->m_lowFilter.process(pInput, pState->m_intermediateBuffer.data(), numSamples);
    pState->m_semiParametricFilter.process(pState->m_intermediateBuffer.data(), pState->m_intermediateBuffer.data(), numSamples);
    pState->m_highFilter.process(pState->m_intermediateBuffer.data(), pOutput, numSamples);

    pState->m_dCenterOld = center;
    pState->m_dGainOld = gain;
    pState->m_dFilterOld = filter;
}
