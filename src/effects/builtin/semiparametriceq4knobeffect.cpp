#include "effects/builtin/semiparametriceq4knobeffect.h"

#include "effects/builtin/semiparametriceqconstants.h"

SemiparametricEQEffect4KnobGroupState::SemiparametricEQEffect4KnobGroupState(
        const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_highFilter(
                  bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kMinCornerHz / bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kLpfHpfQ,
                  true),
          m_semiParametricFilter(
                  bufferParameters.sampleRate(), 1000, mixxx::semiparametriceqs::kSemiparametricQ),
          m_lowFilter(
                  bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kMaxCornerHz / bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kLpfHpfQ,
                  true),
          m_intermediateBuffer(bufferParameters.samplesPerBuffer()),
          m_dHpfOld(0),
          m_dCenterOld(0),
          m_dGainOld(0),
          m_dLpfOld(0) {
}

// static
QString SemiparametricEQEffect4Knob::getId() {
    return QStringLiteral("org.mixxx.effects.semiparametriceq4knob");
}

EffectManifestPointer SemiparametricEQEffect4Knob::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Semiparametric Equalizer (4 knobs)"));
    pManifest->setShortName(QObject::tr("Semiparam 4"));
    pManifest->setAuthor(QStringLiteral("The Mixxx Team"));
    pManifest->setVersion(QStringLiteral("1.0"));
    pManifest->setDescription(
            QObject::tr("A semiparametric EQ effect modeled after the "
                        "PLAYdifferently Model 1 hardware mixer."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC_INVERSE);
    hpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(mixxx::semiparametriceqs::kMaxCornerHz);
    hpf->setMinimum(mixxx::semiparametriceqs::kMinCornerHz);
    hpf->setMaximum(mixxx::semiparametriceqs::kMaxCornerHz);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Gain of the semiparametric EQ"));
    gain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    gain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::DECIBELS);
    gain->setMinimum(0);
    gain->setMaximum(4);
    gain->setDefault(1);

    EffectManifestParameterPointer center = pManifest->addParameter();
    center->setId("center");
    center->setName(QObject::tr("Center"));
    center->setDescription(QObject::tr("Center frequency of the semiparametric EQ"));
    center->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    center->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    center->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    center->setMinimum(70);
    center->setMaximum(7000);
    center->setDefault(1000);

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lpf->setNeutralPointOnScale(1);
    lpf->setMinimum(mixxx::semiparametriceqs::kMinCornerHz);
    lpf->setMaximum(mixxx::semiparametriceqs::kMaxCornerHz);
    lpf->setDefault(mixxx::semiparametriceqs::kMaxCornerHz);

    return pManifest;
}

SemiparametricEQEffect4Knob::SemiparametricEQEffect4Knob(EngineEffect* pEffect)
        : m_pHPF(pEffect->getParameterById("hpf")),
          m_pCenter(pEffect->getParameterById("center")),
          m_pGain(pEffect->getParameterById("gain")),
          m_pLPF(pEffect->getParameterById("lpf")) {
}

void SemiparametricEQEffect4Knob::processChannel(const ChannelHandle& handle,
        SemiparametricEQEffect4KnobGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatureState) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatureState);
    Q_UNUSED(enableState);

    double hpf = m_pHPF->value();
    double center = m_pCenter->value();
    double gain = m_pGain->value();
    double lpf = m_pLPF->value();

    if (center != pState->m_dCenterOld || gain != pState->m_dGainOld) {
        double db = gain - 1.0;
        if (db >= 0) {
            db *= mixxx::semiparametriceqs::kSemiparametricMaxBoostDb;
        } else {
            db *= -mixxx::semiparametriceqs::kSemiparametricMaxCutDb;
        }
        pState->m_semiParametricFilter.setFrequencyCorners(
                bufferParameters.sampleRate(),
                center,
                mixxx::semiparametriceqs::kSemiparametricQ,
                db);
    }
    if (hpf != pState->m_dHpfOld) {
        pState->m_lowFilter.setFrequencyCorners(bufferParameters.sampleRate(),
                hpf,
                mixxx::semiparametriceqs::kLpfHpfQ);
    }
    if (lpf != pState->m_dLpfOld) {
        pState->m_lowFilter.setFrequencyCorners(bufferParameters.sampleRate(),
                lpf,
                mixxx::semiparametriceqs::kLpfHpfQ);
    }

    pState->m_highFilter.process(
            pInput,
            pState->m_intermediateBuffer.data(),
            bufferParameters.samplesPerBuffer());
    pState->m_semiParametricFilter.process(
            pState->m_intermediateBuffer.data(),
            pState->m_intermediateBuffer.data(),
            bufferParameters.samplesPerBuffer());
    pState->m_lowFilter.process(
            pState->m_intermediateBuffer.data(),
            pOutput,
            bufferParameters.samplesPerBuffer());

    pState->m_dHpfOld = hpf;
    pState->m_dCenterOld = center;
    pState->m_dGainOld = gain;
    pState->m_dLpfOld = lpf;
}
