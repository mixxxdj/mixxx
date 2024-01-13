#include "effects/backends/builtin/compressoreffect.h"

// static
QString CompressorEffect::getId() {
    return "org.mixxx.effects.compressor";
}

// static
EffectManifestPointer CompressorEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Compressor"));
    pManifest->setShortName(QObject::tr("Compressor"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription("A single-band compressor effect");
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer clipping = pManifest->addParameter();
    clipping->setId("clipping");
    clipping->setName(QObject::tr("Clipping"));
    clipping->setShortName(QObject::tr("Clipping"));
    clipping->setDescription(QObject::tr("Hard limiter to full scale"));
    clipping->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    clipping->setRange(0, 1, 1);
    clipping->appendStep(qMakePair(QObject::tr("Off"), static_cast<int>(Clipping::ClippingOff)));
    clipping->appendStep(qMakePair(QObject::tr("On"), static_cast<int>(Clipping::ClippingOn)));

    EffectManifestParameterPointer autoMakeUp = pManifest->addParameter();
    autoMakeUp->setId("automakeup");
    autoMakeUp->setName(QObject::tr("Auto Makeup Gain"));
    autoMakeUp->setShortName(QObject::tr("Makeup"));
    autoMakeUp->setDescription(QObject::tr(
            "The AutoMakeup button enables automatic makeup gain to 0 db level"));
    autoMakeUp->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    autoMakeUp->setRange(0, 1, 1);
    autoMakeUp->appendStep(qMakePair(
            QObject::tr("Off"), static_cast<int>(AutoMakeUp::AutoMakeUpOff)));
    autoMakeUp->appendStep(qMakePair(
            QObject::tr("On"), static_cast<int>(AutoMakeUp::AutoMakeUpOn)));

    EffectManifestParameterPointer threshold = pManifest->addParameter();
    threshold->setId("threshold");
    threshold->setName(QObject::tr("Threshold (dBFS)"));
    threshold->setShortName(QObject::tr("Threshold"));
    threshold->setDescription(QObject::tr(
            "The Threshold knob adjusts the level above which the compressor starts attenuating the input signal"));
    threshold->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    threshold->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    threshold->setNeutralPointOnScale(0);
    threshold->setRange(-50, -20, 0);

    EffectManifestParameterPointer ratio = pManifest->addParameter();
    ratio->setId("ratio");
    ratio->setName(QObject::tr("Ratio (:1)"));
    ratio->setShortName(QObject::tr("Ratio"));
    ratio->setDescription(
            QObject::tr("The Ratio knob determines how much the signal is "
                        "attenuated above the chosen threshold. "
                        "For a ratio of 4:1, one dB remains for every 4dB of "
                        "input signal above the threshold. "
                        "At a ratio of 1:1 no compression is happening, as the "
                        "input is exactly the output"));
    ratio->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    ratio->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    ratio->setNeutralPointOnScale(0);
    ratio->setRange(1.0, 4.0, 1000);

    EffectManifestParameterPointer knee = pManifest->addParameter();
    knee->setId("knee");
    knee->setName(QObject::tr("Knee (dBFS)"));
    knee->setShortName(QObject::tr("Knee"));
    knee->setDescription(QObject::tr(
            "The Knee knob is used to achieve a rounder compression curve"));
    knee->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    knee->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    knee->setNeutralPointOnScale(0);
    knee->setRange(0.0, 4.0, 24);

    EffectManifestParameterPointer attack = pManifest->addParameter();
    attack->setId("attack");
    attack->setName(QObject::tr("Attack (ms)"));
    attack->setShortName(QObject::tr("Attack"));
    attack->setDescription(QObject::tr(
            "The Attack knob sets the time that determines how fast the "
            "compression will set in once the signal exceeds the threshold"));
    attack->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    attack->setRange(0, 10, 250);

    EffectManifestParameterPointer release = pManifest->addParameter();
    release->setId("release");
    release->setName(QObject::tr("Release (ms)"));
    release->setShortName(QObject::tr("Release"));
    release->setDescription(
            QObject::tr("The Release knob sets the time that determines how "
                        "fast the compressor will recover from the gain "
                        "reduction once the signal falls under the threshold. "
                        "Depending on the input signal, short release times "
                        "may introduce a 'pumping' effect and/or distortion"));
    release->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    release->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    release->setRange(0, 150, 1500);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Output gain"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(
            QObject::tr("The Output gain knob adjusts the level of the output "
                        "signal after the compression was applied"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setRange(-25, 0, 25);

    return pManifest;
}

CompressorGroupState::CompressorGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          previousStateDB(0),
          previousMakeUpGain(0) {
}

void CompressorEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pThreshold = parameters.value("threshold");
    m_pRatio = parameters.value("ratio");
    m_pKnee = parameters.value("knee");
    m_pAttack = parameters.value("attack");
    m_pRelease = parameters.value("release");
    m_pGain = parameters.value("gain");
    m_pAutoMakeUp = parameters.value("automakeup");
    m_pClipping = parameters.value("clipping");
}

void CompressorEffect::processChannel(
        CompressorGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState);

    SINT numSamples = engineParameters.samplesPerBuffer();

    // Compression
    applyCompression(pState, engineParameters, pInput, pOutput);

    // Auto make up
    if (m_pAutoMakeUp->toInt() == static_cast<int>(AutoMakeUp::AutoMakeUpOn)) {
        applyAutoMakeUp(pState, pOutput, numSamples);
    }

    // Output gain
    CSAMPLE gainParamDB = static_cast<CSAMPLE>(m_pGain->value());
    SampleUtil::applyGain(pOutput, db2ratio(gainParamDB), numSamples);

    // Clipping
    if (m_pClipping->toInt() == static_cast<int>(Clipping::ClippingOn)) {
        SampleUtil::applyClamp(pOutput, numSamples);
    }
}

void CompressorEffect::applyAutoMakeUp(CompressorGroupState* pState,
        CSAMPLE* pOutput,
        const SINT& numSamples) {
    CSAMPLE makeUpStateDB = pState->previousMakeUpGain;
    CSAMPLE maxSample = SampleUtil::maxAbsAmplitude(pOutput, numSamples);
    if (maxSample > 0) {
        CSAMPLE maxSampleDB = ratio2db(maxSample);
        CSAMPLE minGainReductionDB = -maxSampleDB + kMakeUpTarget;
        makeUpStateDB = kMakeUpAttackCoeff * minGainReductionDB +
                (1 - kMakeUpAttackCoeff) * makeUpStateDB;
        CSAMPLE levelDB = makeUpStateDB + maxSampleDB;
        // logarithmic smoothing
        if (levelDB > -1.0) {
            makeUpStateDB = log10(levelDB + 2) - 1 - maxSampleDB;
        }

        pState->previousMakeUpGain = makeUpStateDB;
        SampleUtil::applyGain(pOutput, db2ratio(makeUpStateDB), numSamples);
    }
}

void CompressorEffect::applyCompression(CompressorGroupState* pState,
        const mixxx::EngineParameters& engineParameters,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput) {
    CSAMPLE thresholdParam = static_cast<CSAMPLE>(m_pThreshold->value());
    CSAMPLE ratioParam = static_cast<CSAMPLE>(m_pRatio->value());
    CSAMPLE kneeParam = static_cast<CSAMPLE>(m_pKnee->value());
    CSAMPLE kneeHalf = kneeParam / 2.0f;
    CSAMPLE attackCoeff = exp(-1000.0 / (m_pAttack->value() * engineParameters.sampleRate()));
    CSAMPLE releaseCoeff = exp(-1000.0 / (m_pRelease->value() * engineParameters.sampleRate()));

    CSAMPLE stateDB = pState->previousStateDB;
    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    for (SINT i = 0; i < numSamples; i += channelCount) {
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        if (maxSample == CSAMPLE_ZERO) {
            pOutput[i] = CSAMPLE_ZERO;
            pOutput[i + 1] = CSAMPLE_ZERO;
            continue;
        }

        CSAMPLE maxSampleDB = ratio2db(maxSample);
        CSAMPLE overDB = maxSampleDB - thresholdParam;
        if (overDB <= -kneeHalf) {
            overDB = 0.0f;
        } else if (overDB > -kneeHalf && overDB <= kneeHalf) {
            overDB = 0.5f * (overDB + kneeHalf) * (overDB + kneeHalf) / kneeParam;
        }
        CSAMPLE compressedDB = overDB * (1.0 / ratioParam - 1.0);

        // attack/release
        if (compressedDB < stateDB) {
            stateDB = compressedDB + attackCoeff * (stateDB - compressedDB);
        } else {
            stateDB = compressedDB + releaseCoeff * (stateDB - compressedDB);
        }

        CSAMPLE gain = db2ratio(stateDB);
        pOutput[i] = pInput[i] * gain;
        pOutput[i + 1] = pInput[i + 1] * gain;
    }
    pState->previousStateDB = stateDB;
}
