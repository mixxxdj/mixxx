#include "effects/backends/builtin/compressoreffect.h"
#include <Windows.h>

// static
QString CompressorEffect::getId() {
    return "org.mixxx.effects.compressor";
}

//static
EffectManifestPointer CompressorEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest);
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Compressor"));
    pManifest->setShortName(QObject::tr("Compressor"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            "A single-band compressor effect");
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer clipping = pManifest->addParameter();
    clipping->setId("clipping");
    clipping->setName(QObject::tr("Clipping"));
    clipping->setShortName(QObject::tr("Clipping"));
    clipping->setDescription(QObject::tr("Hard limiter to 0 db"));
    clipping->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    clipping->setRange(0, 1, 1);
    clipping->appendStep(qMakePair(QObject::tr("Off"), Clipping::ClippingOff));
    clipping->appendStep(qMakePair(QObject::tr("On"), Clipping::ClippingOn));

    EffectManifestParameterPointer autoMakeUp = pManifest->addParameter();
    autoMakeUp->setId("automakeup");
    autoMakeUp->setName(QObject::tr("Auto Make Up Gain"));
    autoMakeUp->setShortName(QObject::tr("Make Up"));
    autoMakeUp->setDescription(QObject::tr(
            "Auto make up gain to 0 db level"));
    autoMakeUp->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    autoMakeUp->setRange(0, 1, 1);
    autoMakeUp->appendStep(qMakePair(QObject::tr("Off"), AutoMakeUp::AutoMakeUpOff));
    autoMakeUp->appendStep(qMakePair(QObject::tr("On"), AutoMakeUp::AutoMakeUpOn));


    //TODO description
    EffectManifestParameterPointer threshold = pManifest->addParameter();
    threshold->setId("threshold");
    threshold->setName(QObject::tr("Threshold (dB)"));
    threshold->setShortName(QObject::tr("Threshold"));
    threshold->setDescription(QObject::tr(
            "The amount of amplification "
            "applied to the audio signal. At higher levels the audio will be more distored."));
    threshold->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    //threshold->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    threshold->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    threshold->setNeutralPointOnScale(0);
    threshold->setRange(-50, -20, 0);

    EffectManifestParameterPointer ratio = pManifest->addParameter();
    ratio->setId("ratio");
    ratio->setName(QObject::tr("Ratio (:1)"));
    ratio->setShortName(QObject::tr("Ratio"));
    ratio->setDescription(QObject::tr(
            "The amount of ratio."));
    ratio->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    //ratio->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    ratio->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    ratio->setNeutralPointOnScale(0);
    ratio->setRange(1.0, 4.0, 20);

    EffectManifestParameterPointer knee = pManifest->addParameter();
    knee->setId("knee");
    knee->setName(QObject::tr("Knee (dB)"));
    knee->setShortName(QObject::tr("Knee"));
    knee->setDescription(QObject::tr(
            "The amount of knee."));
    knee->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    //knee->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    knee->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    knee->setNeutralPointOnScale(0);
    knee->setRange(0.0, 4.0, 24);

    EffectManifestParameterPointer attack = pManifest->addParameter();
    attack->setId("attack");
    attack->setName(QObject::tr("Attack (ms)"));
    attack->setShortName(QObject::tr("Attack"));
    attack->setDescription(QObject::tr(
            "Attack"));
    attack->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    attack->setRange(0, 30, 250);

    EffectManifestParameterPointer release = pManifest->addParameter();
    release->setId("release");
    release->setName(QObject::tr("Release (ms)"));
    release->setShortName(QObject::tr("Release"));
    release->setDescription(QObject::tr(
            "Release"));
    release->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    release->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    release->setRange(0, 150, 2000);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Make up gain"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr(
            "Gain"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setRange(-25, 0, 25);

    return pManifest;
}

CompressorGroupState::CompressorGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          samplerate(engineParameters.sampleRate()),
          previousMakeUpGain(0),
          previousStateDB(0) {
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

    // TODO test: Marc Benjamin, Zana - Edge of Paradise; Cassette - Tell Me Why; Adele - Skyfall
    // url: https://github.com/p-hlp/CTAGDRC

    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    SINT numFrames = engineParameters.framesPerBuffer();

    CSAMPLE thresholdParam = static_cast<CSAMPLE>(m_pThreshold->value());
    CSAMPLE ratioParam = static_cast<CSAMPLE>(m_pRatio->value());
    //CSAMPLE kneeParam = static_cast<CSAMPLE>(m_pKnee->value());
    //CSAMPLE kneeHalf = kneeParam / 2.0f;
    CSAMPLE attackCoeff = exp(-1000.0 / (m_pAttack->value() * pState->samplerate));
    CSAMPLE releaseCoeff = exp(-1000.0 / (m_pRelease->value() * pState->samplerate));

    CSAMPLE stateDB = pState->previousStateDB;

    for (SINT i = 0; i < numSamples; i += channelCount) {
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        if (maxSample == CSAMPLE_ZERO) {
            pOutput[i] = CSAMPLE_ZERO;
            pOutput[i + 1] = CSAMPLE_ZERO;
            continue;
        }

        CSAMPLE maxSampleDB = ratio2db(maxSample);
        CSAMPLE overDB = maxSampleDB - thresholdParam;
        //if (overDB <= kneeHalf) {
        //    overDB = 0.0f;
        //} else if (overDB > -kneeHalf && overDB <= kneeHalf) {
        //    overDB = 0.5f * (overDB + kneeHalf) * (overDB + kneeHalf) / kneeParam;
        //}

        // atack/release
        if (overDB > stateDB) {
            stateDB = overDB + attackCoeff * (stateDB - overDB);
        } else {
            stateDB = overDB + releaseCoeff * (stateDB - overDB);
        }


        overDB = stateDB;
        CSAMPLE gain = db2ratio(overDB * (1.0 / ratioParam - 1.0));
        pOutput[i] = pInput[i] * gain;
        pOutput[i + 1] = pInput[i + 1] * gain;
    }
    pState->previousStateDB = stateDB;

    if (m_pAutoMakeUp->toInt() == AutoMakeUpOn) { 
        CSAMPLE makeUpState = pState->previousMakeUpGain;
        CSAMPLE maxSample = SampleUtil::maxAbsAmplitude(pOutput, numSamples);
        if (maxSample > CSAMPLE_ZERO) {
            CSAMPLE minGainReduction = (1 / maxSample) * makeUpCoeff;
            makeUpState = makeUpAttackCoeff * minGainReduction + (1 - makeUpAttackCoeff) * makeUpState;
            pState->previousMakeUpGain = makeUpState;

            SampleUtil::applyGain(pOutput, makeUpState, numSamples);
        }
    }

    CSAMPLE gainParamDB = static_cast<CSAMPLE>(m_pGain->value());
    SampleUtil::applyGain(pOutput, db2ratio(gainParamDB), numSamples);

    
    if (m_pClipping->toInt() == ClippingOn) {
        SampleUtil::copyClampBuffer(pOutput, pOutput, numSamples);
    }


    //std::string msg2 = std::string("makeUpStateDB: ") + std::to_string(makeUpStateDB) + std::string(" stateDB: ") + std::to_string(stateDB) + std::string("\n");
    //OutputDebugStringA(msg2.c_str());

    /*
    if (m_pAutoMakeUp->toInt() == On) {
         CSAMPLE sum = CSAMPLE_ZERO;
        for (SINT i = 0; i < numSamples; i += channelCount) {
            sum += std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        }
        CSAMPLE max = SampleUtil::maxAbsAmplitude(pOutput, numSamples);
        if (max != CSAMPLE_ZERO) {
            CSAMPLE averageDB = -ratio2db(max) - 2;
            CSAMPLE makeUpSatetDB = pState->previousMakeUpGain;

            makeUpSatetDB = averageDB + makeUpCoeff * (makeUpSatetDB - averageDB);
            std::string msg = std::string("makeUpCoeff: ") + std::to_string(makeUpCoeff) + std::string(" averageDB: ") + std::to_string(averageDB) + std::string(" makeUpSatetDB: ") + std::to_string(makeUpSatetDB) + std::string("\n");
            OutputDebugStringA(msg.c_str());

            gainParamDB += averageDB;
            pState->previousMakeUpGain = makeUpSatetDB;
        }
    }


    SampleUtil::applyGain(pOutput, db2ratio(gainParamDB), numSamples);
    */
}
