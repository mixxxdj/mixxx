#include "effects/backends/builtin/bandpassreverbeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"
#include <QDebug>
#include <vector>

QString BandpassReverbEffect::getId() {
    return "org.mixxx.effects.bandpassreverb";
}

// static
EffectManifestPointer BandpassReverbEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setAddDryToWet(true);
    pManifest->setEffectRampsFromDry(true);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Bandpass Reverb"));
    pManifest->setAuthor("The Mixxx Team, CAPS Plugins");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Reverb effect followed by a band-pass filter for shaping the reverberated signal."));

    EffectManifestParameterPointer decay = pManifest->addParameter();
    decay->setId("decay");
    decay->setName(QObject::tr("Decay"));
    decay->setShortName(QObject::tr("Decay"));
    decay->setDescription(QObject::tr(
            "Lower decay values cause reverberations to fade out more quickly."));
    decay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    decay->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    decay->setRange(0, 0.5, 1);

    EffectManifestParameterPointer damping = pManifest->addParameter();
    damping->setId("damping");
    damping->setName(QObject::tr("Damping"));
    damping->setShortName(QObject::tr("Damping"));
    damping->setDescription(
            QObject::tr("Higher damping values cause high frequencies to decay "
                        "more quickly than low frequencies."));
    damping->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    damping->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    damping->setRange(0, 0, 1);

    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
            "How much of the signal to send in to the effect"));
    send->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    send->setDefaultLinkInversion(EffectManifestParameter::LinkInversion::NotInverted);
    send->setRange(0, 0, 1);

    EffectManifestParameterPointer hpCutoff = pManifest->addParameter();
    hpCutoff->setId("hp_cutoff");
    hpCutoff->setName(QObject::tr("HP Cutoff"));
    hpCutoff->setShortName(QObject::tr("HP"));
    hpCutoff->setDescription(
            QObject::tr("High-pass cutoff frequency"));
    hpCutoff->setValueScaler(
            EffectManifestParameter::ValueScaler::Logarithmic);
    hpCutoff->setUnitsHint(
            EffectManifestParameter::UnitsHint::Unknown);
    hpCutoff->setRange(20, 200, 18000);

    EffectManifestParameterPointer lpCutoff = pManifest->addParameter();
    lpCutoff->setId("lp_cutoff");
    lpCutoff->setName(QObject::tr("LP Cutoff"));
    lpCutoff->setShortName(QObject::tr("LP"));
    lpCutoff->setDescription(
            QObject::tr("Low-pass cutoff frequency"));
    lpCutoff->setValueScaler(
            EffectManifestParameter::ValueScaler::Logarithmic);
    lpCutoff->setUnitsHint(
            EffectManifestParameter::UnitsHint::Unknown);
    lpCutoff->setRange(20, 5000, 20000);

    EffectManifestParameterPointer postFilter = pManifest->addParameter();
    postFilter->setId("post_filter");
    postFilter->setName(QObject::tr("Post Filter"));
    postFilter->setShortName(QObject::tr("Post Filter"));
    postFilter->setDescription(
            QObject::tr("Apply band-pass filter after the reverb"));
    postFilter->setValueScaler(
            EffectManifestParameter::ValueScaler::Toggle);
    postFilter->setUnitsHint(
            EffectManifestParameter::UnitsHint::Unknown);
    postFilter->setRange(0, 0, 1);

    EffectManifestParameterPointer filterOrder = pManifest->addParameter();
    filterOrder->setId("filter_order");
    filterOrder->setName(QObject::tr("Filter Order"));
    filterOrder->setShortName(QObject::tr("Order"));
    filterOrder->setDescription(QObject::tr(
        "Selects Butterworth filter steepness (2/4/6/8)."));
    filterOrder->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    //filterOrder->appendStep(qMakePair(QObject::tr("2"), 2));
    //filterOrder->appendStep(qMakePair(QObject::tr("4"), 4));
    //filterOrder->appendStep(qMakePair(QObject::tr("6"), 6));
    //filterOrder->appendStep(qMakePair(QObject::tr("8"), 8));
    filterOrder->setRange(1, 4, 4);

    return pManifest;
}

void BandpassReverbEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDecayParameter = parameters.value("decay");
    m_pDampingParameter = parameters.value("damping");
    m_pSendParameter = parameters.value("send_amount");
    m_pHPCutoffParameter = parameters.value("hp_cutoff");
    m_pLPCutoffParameter = parameters.value("lp_cutoff");
    m_pPostFilterParameter = parameters.value("post_filter");
    m_pFilterOrderParameter = parameters.value("filter_order");
    
}

void BandpassReverbEffect::processChannel(
        BandpassReverbGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures){
  
    Q_UNUSED(groupFeatures);

    const auto decay = static_cast<sample_t>(m_pDecayParameter->value());
    const auto damping = static_cast<sample_t>(m_pDampingParameter->value());
    const auto sendCurrent = static_cast<sample_t>(m_pSendParameter->value());
    const bool postFilter = m_pPostFilterParameter->toBool();
    const int filterOrder = static_cast<int>(m_pFilterOrderParameter->value());

    if (pState->sampleRate != engineParameters.sampleRate()) {
        pState->reverb.setSamplerate(engineParameters.sampleRate());
        pState->sampleRate = engineParameters.sampleRate();
    }

    if (enableState == EffectEnableState::Enabling) {
        pState->reverb.activate();
    }

    const sample_t nyquist = static_cast<sample_t>(engineParameters.sampleRate() * 0.5);

    sample_t lowFreq = static_cast<sample_t>(m_pHPCutoffParameter->value());

    sample_t highFreq = static_cast<sample_t>(m_pLPCutoffParameter->value());

    lowFreq = std::clamp(lowFreq, static_cast<sample_t>(20.0), nyquist - static_cast<sample_t>(100.0));

    highFreq = std::clamp(highFreq, static_cast<sample_t>(20.0), nyquist - static_cast<sample_t>(100.0));

    if (highFreq <= lowFreq) { highFreq = std::min(lowFreq + static_cast<sample_t>(50.0), nyquist - static_cast<sample_t>(1.0));
    }

    switch (filterOrder) {
        case 1:
        pState->butter2.setFrequencyCorners(
                engineParameters.sampleRate(),
                lowFreq,
                highFreq);
        break;
        case 2:
        pState->butter4.setFrequencyCorners(
                engineParameters.sampleRate(),
                lowFreq,
                highFreq);
        break;
        case 3:
        pState->butter6.setFrequencyCorners(
                engineParameters.sampleRate(),
                lowFreq,
                highFreq);
        break;
        default:
        pState->butter8.setFrequencyCorners(
                engineParameters.sampleRate(),
                lowFreq,
                highFreq);
        break;
        }

auto processFilter = [&](const CSAMPLE* in, CSAMPLE* out) {
    switch (filterOrder) {
    case 1:
        pState->butter2.process(
                in, out,
                engineParameters.samplesPerBuffer());
        break;
    case 2:
        pState->butter4.process(
                in, out,
                engineParameters.samplesPerBuffer());
        break;
    case 3:
        pState->butter6.process(
                in, out,
                engineParameters.samplesPerBuffer());
        break;
    default:
        pState->butter8.process(
                in, out,
                engineParameters.samplesPerBuffer());
        break;
    }
};

if (!postFilter) {
    // PRE FILTER
    std::vector<CSAMPLE> filteredBuffer(
            engineParameters.samplesPerBuffer());

    processFilter(
        pInput,
        filteredBuffer.data());

    pState->reverb.processBuffer(
            filteredBuffer.data(),
            pOutput,
            engineParameters.samplesPerBuffer(),
            1.0f,
            decay,
            damping,
            sendCurrent,
            pState->sendPrevious);

} else {
    // POST FILTER
    std::vector<CSAMPLE> reverbBuffer(
            engineParameters.samplesPerBuffer());

    pState->reverb.processBuffer(
            pInput,
            reverbBuffer.data(),
            engineParameters.samplesPerBuffer(),
            1.0f,
            decay,
            damping,
            sendCurrent,
            pState->sendPrevious);

    processFilter(
    reverbBuffer.data(),
    pOutput);
}

    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::applyRampingGain(
            pOutput,
            static_cast<sample_t>(1.0),
            static_cast<sample_t>(0.0),
            engineParameters.samplesPerBuffer()
        );
        pState->sendPrevious = static_cast<sample_t>(0.0);
    } else {
        pState->sendPrevious = sendCurrent;
    }
}
