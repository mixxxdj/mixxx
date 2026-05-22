
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
    pManifest->setAddDryToWet(false);
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

    EffectManifestParameterPointer bandwidth = pManifest->addParameter();
    bandwidth->setId("bandwidth");
    bandwidth->setName(QObject::tr("Bandwidth"));
    bandwidth->setShortName(QObject::tr("BW"));
    bandwidth->setDescription(QObject::tr(
            "Bandwidth of the low pass filter at the input.\n"
            "Higher values result in less attenuation of high frequencies."));
    bandwidth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bandwidth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bandwidth->setRange(0, 1, 1);

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

    EffectManifestParameterPointer bpFreq = pManifest->addParameter();
    bpFreq->setId("bp_freq");
    bpFreq->setName(QObject::tr("BP Frequency"));
    bpFreq->setShortName(QObject::tr("BPFreq"));
    bpFreq->setDescription(QObject::tr("Center frequency of band-pass filter"));
    bpFreq->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    bpFreq->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bpFreq->setRange(200, 1000, 5000);

    EffectManifestParameterPointer bpQ = pManifest->addParameter();
    bpQ->setId("bp_q");
    bpQ->setName(QObject::tr("Bandwidth"));
    bpQ->setShortName(QObject::tr("Width"));
    bpQ->setDescription(QObject::tr("Controls width of the band-pass filter"));
    bpQ->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bpQ->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bpQ->setRange(0.1, 0.707, 10);

    EffectManifestParameterPointer order = pManifest->addParameter();
    order->setId("filter_order");
    order->setName(QObject::tr("Filter Order"));
    order->setShortName(QObject::tr("Order"));
    order->setDescription(QObject::tr("Controls steepness of band-pass filter"));
    order->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    order->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    order->setRange(1, 1, 4);

    return pManifest;
}

void BandpassReverbEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDecayParameter = parameters.value("decay");
    m_pBandWidthParameter = parameters.value("bandwidth");
    m_pDampingParameter = parameters.value("damping");
    m_pSendParameter = parameters.value("send_amount");
    m_pBPFreqParameter = parameters.value("bp_freq");
    m_pBPQParameter = parameters.value("bp_q");
    m_pOrderParameter = parameters.value("filter_order");
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
    const auto bandwidth = static_cast<sample_t>(m_pBandWidthParameter->value());
    const auto damping = static_cast<sample_t>(m_pDampingParameter->value());
    const auto sendCurrent = static_cast<sample_t>(m_pSendParameter->value());

    if (pState->sampleRate != engineParameters.sampleRate()) {
        pState->reverb.setSamplerate(engineParameters.sampleRate());
        pState->sampleRate = engineParameters.sampleRate();
    }


    if (enableState == EffectEnableState::Enabling) {
        pState->reverb.activate();
    }

 
    sample_t freq = static_cast<sample_t>(m_pBPFreqParameter->value());
    sample_t q = static_cast<sample_t>(m_pBPQParameter->value());

    sample_t bandwidthHz = freq / q;

    sample_t lowFreq = freq - bandwidthHz * 0.5;
    sample_t highFreq = freq + bandwidthHz * 0.5;

    if (lowFreq < 20.0) {
        lowFreq = 20.0;
    }

    if (highFreq > engineParameters.sampleRate() / 2.0 - 100.0) {
        highFreq = engineParameters.sampleRate() / 2.0 - 100.0;
    }

    pState->butterworthBP.setFrequencyCorners(
            engineParameters.sampleRate(),
            lowFreq,
            highFreq);


 std::vector<CSAMPLE> filteredBuffer(engineParameters.samplesPerBuffer());
std::vector<CSAMPLE> tempBuffer(engineParameters.samplesPerBuffer());

const int order = static_cast<int>(m_pOrderParameter->value());

pState->butterworthBP.process(
        pInput,
        filteredBuffer.data(),
        engineParameters.samplesPerBuffer());

for (int stage = 1; stage < order; ++stage) {

    pState->butterworthBP.process(
            filteredBuffer.data(),
            tempBuffer.data(),
            engineParameters.samplesPerBuffer());

    filteredBuffer.swap(tempBuffer);
}





    pState->reverb.processBuffer(
            filteredBuffer.data(),
            pOutput,
            engineParameters.samplesPerBuffer(),
            bandwidth,
            decay,
            damping,
            sendCurrent,
            pState->sendPrevious);


    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::applyRampingGain(pOutput, 1.0, 0.0, engineParameters.samplesPerBuffer());
        pState->sendPrevious = 0;
    } else {
        pState->sendPrevious = sendCurrent;
    }
}
