#include "semiparametriceq4knobeffect.h"

#include "semiparametriceqconstants.h"

using namespace mixxx::semiparametriceqs;

SemiparametricEQEffect4KnobGroupState::SemiparametricEQEffect4KnobGroupState(
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

    EffectManifestParameterPointer filter = pManifest->addParameter();
    filter->setId(QStringLiteral("filter"));
    filter->setName(QObject::tr("Filter"));
    filter->setDescription(
            QObject::tr("Bipolar filter knob. Controls corner frequency ratio "
                        "of the high pass filter on the left and corner "
                        "frequency of the low pass filter on the right."));
    filter->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
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

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId(QStringLiteral("hpf"));
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setRange(kMaxCornerHz, kMaxCornerHz, kMinCornerHz);
    hpf->setNeutralPointOnScale(0.0);

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId(QStringLiteral("lpf"));
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setRange(kMinCornerHz, kMaxCornerHz, kMaxCornerHz);
    lpf->setNeutralPointOnScale(1);

    return pManifest;
}

void SemiparametricEQEffect4Knob::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pHPF = parameters.value(QStringLiteral("hpf"));
    m_pCenter = parameters.value(QStringLiteral("center"));
    m_pGain = parameters.value(QStringLiteral("gain"));
    m_pLPF = parameters.value(QStringLiteral("lpf"));
}

void SemiparametricEQEffect4Knob::processChannel(
        SemiparametricEQEffect4KnobGroupState* channelState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);

    double hpf = m_pHPF->value();
    double center = m_pCenter->value();
    double gain = m_pGain->value();
    double lpf = m_pLPF->value();

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
    if (hpf != channelState->m_dHpfOld) {
        channelState->m_lowFilter.setFrequencyCorners(engineParameters.sampleRate(), hpf, kLpfHpfQ);
    }
    if (lpf != channelState->m_dLpfOld) {
        channelState->m_lowFilter.setFrequencyCorners(engineParameters.sampleRate(), lpf, kLpfHpfQ);
    }

    channelState->m_highFilter.process(
            pInput,
            channelState->m_intermediateBuffer.data(),
            engineParameters.samplesPerBuffer());
    channelState->m_semiParametricFilter.process(
            channelState->m_intermediateBuffer.data(),
            channelState->m_intermediateBuffer.data(),
            engineParameters.samplesPerBuffer());
    channelState->m_lowFilter.process(
            channelState->m_intermediateBuffer.data(),
            pOutput,
            engineParameters.samplesPerBuffer());

    channelState->m_dHpfOld = hpf;
    channelState->m_dCenterOld = center;
    channelState->m_dGainOld = gain;
    channelState->m_dLpfOld = lpf;
}
