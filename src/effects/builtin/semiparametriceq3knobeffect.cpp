#include "effects/builtin/semiparametriceq3knobeffect.h"

#include "effects/builtin/semiparametriceqconstants.h"

SemiparametricEQEffect3KnobGroupState::SemiparametricEQEffect3KnobGroupState(
        const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_highFilter(bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kMinCornerHz /
                          bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kLpfHpfQ,
                  true),
          m_semiParametricFilter(bufferParameters.sampleRate(),
                  1000,
                  mixxx::semiparametriceqs::kSemiparametricQ),
          m_lowFilter(bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kMaxCornerHz /
                          bufferParameters.sampleRate(),
                  mixxx::semiparametriceqs::kLpfHpfQ,
                  true),
          m_intermediateBuffer(bufferParameters.samplesPerBuffer()),
          m_filterBehavior(mixxx::semiparametriceqs::kMinCornerHz,
                  mixxx::semiparametriceqs::kMaxCornerHz,
                  -40),
          m_dCenterOld(0),
          m_dGainOld(0),
          m_dFilterOld(0) {
}

// static
QString SemiparametricEQEffect3Knob::getId() {
    return QStringLiteral("org.mixxx.effects.semiparametriceq3knob");
}

EffectManifestPointer SemiparametricEQEffect3Knob::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Semiparametric Equalizer (3 knobs)"));
    pManifest->setShortName(QObject::tr("Semiparam 3"));
    pManifest->setAuthor(QStringLiteral("The Mixxx Team"));
    pManifest->setVersion(QStringLiteral("1.0"));
    pManifest->setDescription(
            QObject::tr("A semiparametric EQ effect modeled after the "
                        "PLAYdifferently Model 1 hardware mixer."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setIsMixingEQ(true);

    EffectManifestParameterPointer filter = pManifest->addParameter();
    filter->setId("filter");
    filter->setName(QObject::tr("Filter"));
    filter->setDescription(
            QObject::tr("Bipolar filter knob. Controls corner frequency ratio "
                        "of the high pass filter on the left and corner "
                        "frequency of the low pass filter on the right."));
    filter->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    filter->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    filter->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    filter->setMinimum(0);
    filter->setMaximum(1);
    filter->setDefault(0.5);

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

    return pManifest;
}

SemiparametricEQEffect3Knob::SemiparametricEQEffect3Knob(EngineEffect* pEffect)
        : m_pCenter(pEffect->getParameterById("center")),
          m_pGain(pEffect->getParameterById("gain")),
          m_pFilter(pEffect->getParameterById("filter")) {
}

void SemiparametricEQEffect3Knob::processChannel(const ChannelHandle& handle,
        SemiparametricEQEffect3KnobGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
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
    if (filter != pState->m_dFilterOld) {
        double lpf = pState->m_filterBehavior.parameterToValue(
                             std::min((filter * 2.0), 1.0)) /
                bufferParameters.sampleRate();
        double hpf = pState->m_filterBehavior.parameterToValue(
                             std::max((filter - 0.5) * 2.0, 0.0)) /
                bufferParameters.sampleRate();
        pState->m_lowFilter.setFrequencyCorners(1, lpf, mixxx::semiparametriceqs::kLpfHpfQ);
        pState->m_highFilter.setFrequencyCorners(1, hpf, mixxx::semiparametriceqs::kLpfHpfQ);
    }

    pState->m_lowFilter.process(
            pInput,
            pState->m_intermediateBuffer.data(),
            bufferParameters.samplesPerBuffer());
    pState->m_semiParametricFilter.process(
            pState->m_intermediateBuffer.data(),
            pState->m_intermediateBuffer.data(),
            bufferParameters.samplesPerBuffer());
    pState->m_highFilter.process(
            pState->m_intermediateBuffer.data(),
            pOutput,
            bufferParameters.samplesPerBuffer());

    pState->m_dCenterOld = center;
    pState->m_dGainOld = gain;
    pState->m_dFilterOld = filter;
}
