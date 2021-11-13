#include "semiparametriceq3knobeffect.h"

#include "semiparametriceqconstants.h"

using namespace mixxx::semiparametriceqs;

SemiparametricEQEffect3KnobGroupState::SemiparametricEQEffect3KnobGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_highFilter(
                  engineParameters.sampleRate(),
                  kMinCornerHz / engineParameters.sampleRate(),
                  kLpfHpfQ,
                  true),
          m_semiParametricFilter(
                  engineParameters.sampleRate(), 1000, kSemiparametricQ),
          m_lowFilter(
                  engineParameters.sampleRate(),
                  kMaxCornerHz / engineParameters.sampleRate(),
                  kLpfHpfQ,
                  true),
          m_intermediateBuffer(engineParameters.samplesPerBuffer()),
          m_filterBehavior(kMinCornerHz, kMaxCornerHz, -40),
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
    filter->setId(QStringLiteral("filter"));
    filter->setName(QObject::tr("Filter"));
    filter->setDescription(
            QObject::tr("Bipolar filter knob. Controls corner frequency ratio "
                        "of the high pass filter on the left and corner "
                        "frequency of the low pass filter on the right."));
    filter->setRange(0, 0.5, 1);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId(QStringLiteral("gain"));
    gain->setName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Gain of the semiparametric EQ"));
    gain->setRange(0, 1, 4);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibels);

    EffectManifestParameterPointer center = pManifest->addParameter();
    center->setId(QStringLiteral("center"));
    center->setName(QObject::tr("Center"));
    center->setDescription(QObject::tr("Center frequency of the semiparametric EQ"));
    center->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    center->setRange(70, 1000, 7000);

    return pManifest;
}

void SemiparametricEQEffect3Knob::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pCenter = parameters.value(QStringLiteral("center"));
    m_pGain = parameters.value(QStringLiteral("gain"));
    m_pFilter = parameters.value(QStringLiteral("filter"));
}

void SemiparametricEQEffect3Knob::processChannel(
        SemiparametricEQEffect3KnobGroupState* channelState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    double center = m_pCenter->value();
    double gain = m_pGain->value();
    double filter = m_pFilter->value();

    if (center != channelState->m_dCenterOld || gain != channelState->m_dGainOld) {
        double db = gain - 1.0;
        if (db >= 0) {
            db *= kSemiparametricMaxBoostDb;
        } else {
            db *= -kSemiparametricMaxCutDb;
        }
        channelState->m_semiParametricFilter.setFrequencyCorners(
                engineParameters.sampleRate(), center, kSemiparametricQ, db);
    }
    if (filter != channelState->m_dFilterOld) {
        double lpf = channelState->m_filterBehavior.parameterToValue(
                             std::min((filter * 2.0), 1.0)) /
                engineParameters.sampleRate();
        double hpf = channelState->m_filterBehavior.parameterToValue(
                             std::max((filter - 0.5) * 2.0, 0.0)) /
                engineParameters.sampleRate();
        channelState->m_lowFilter.setFrequencyCorners(1, lpf, kLpfHpfQ);
        channelState->m_highFilter.setFrequencyCorners(1, hpf, kLpfHpfQ);
    }

    channelState->m_lowFilter.process(
            pInput,
            channelState->m_intermediateBuffer.data(),
            engineParameters.samplesPerBuffer());
    channelState->m_semiParametricFilter.process(
            channelState->m_intermediateBuffer.data(),
            channelState->m_intermediateBuffer.data(),
            engineParameters.samplesPerBuffer());
    channelState->m_highFilter.process(
            channelState->m_intermediateBuffer.data(),
            pOutput,
            engineParameters.samplesPerBuffer());

    channelState->m_dCenterOld = center;
    channelState->m_dGainOld = gain;
    channelState->m_dFilterOld = filter;
}
