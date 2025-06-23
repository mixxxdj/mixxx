#include "effects/backends/builtin/compressoreffect.h"

namespace {
constexpr CSAMPLE_GAIN kMakeUpAttackCoeff = 0.03f;
constexpr double defaultAttackMs = 1;
constexpr double defaultReleaseMs = 300;
constexpr CSAMPLE_GAIN defaultThresholdDB = -20;

double calculateBallistics(double paramMs, const mixxx::EngineParameters& engineParameters) {
    return exp(-1000.0 / (paramMs * engineParameters.sampleRate()));
}

} // anonymous namespace

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

    EffectManifestParameterPointer autoMakeUp = pManifest->addParameter();
    autoMakeUp->setId("automakeup");
    autoMakeUp->setName(QObject::tr("Auto Makeup Gain"));
    autoMakeUp->setShortName(QObject::tr("Makeup"));
    autoMakeUp->setDescription(QObject::tr(
            "The Auto Makeup button enables automatic gain adjustment to keep "
            "the input signal \nand the processed output signal as close as "
            "possible in perceived loudness"));
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
    threshold->setDescription(
            QObject::tr("The Threshold knob adjusts the level above which the "
                        "compressor starts attenuating the input signal"));
    threshold->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    threshold->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    threshold->setNeutralPointOnScale(0);
    threshold->setRange(-50, defaultThresholdDB, 0);

    EffectManifestParameterPointer ratio = pManifest->addParameter();
    ratio->setId("ratio");
    ratio->setName(QObject::tr("Ratio (:1)"));
    ratio->setShortName(QObject::tr("Ratio"));
    ratio->setDescription(
            QObject::tr("The Ratio knob determines how much the signal is "
                        "attenuated above the chosen threshold.\n"
                        "For a ratio of 4:1, one dB remains for every four dB of "
                        "input signal above the threshold.\n"
                        "At a ratio of 1:1 no compression is happening, as the "
                        "input is exactly the output."));
    ratio->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    ratio->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    ratio->setNeutralPointOnScale(0);
    ratio->setRange(1.0, 6.0, 1000);

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
            "compression \nwill set in once the signal exceeds the threshold"));
    attack->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    attack->setRange(0, defaultAttackMs, 250);

    EffectManifestParameterPointer release = pManifest->addParameter();
    release->setId("release");
    release->setName(QObject::tr("Release (ms)"));
    release->setShortName(QObject::tr("Release"));
    release->setDescription(
            QObject::tr("The Release knob sets the time that determines how "
                        "fast the compressor will recover from the gain\n"
                        "reduction once the signal falls under the threshold. "
                        "Depending on the input signal, short release times\n"
                        "may introduce a 'pumping' effect and/or distortion."));
    release->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    release->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    release->setRange(0, defaultReleaseMs, 1500);

    EffectManifestParameterPointer level = pManifest->addParameter();
    level->setId("level");
    level->setName(QObject::tr("Level"));
    level->setShortName(QObject::tr("Level"));
    level->setDescription(
            QObject::tr("The Level knob adjusts the level of the output "
                        "signal after the compression was applied"));
    level->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    level->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    level->setRange(-25, 0, 25);

    return pManifest;
}

CompressorGroupState::CompressorGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          previousStateDB(0),
          previousAttackParamMs(defaultAttackMs),
          previousAttackCoeff(calculateBallistics(defaultAttackMs, engineParameters)),
          previousReleaseParamMs(defaultReleaseMs),
          previousReleaseCoeff(calculateBallistics(defaultReleaseMs, engineParameters)),
          previousMakeUpGain(1) {
}

void CompressorEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pThreshold = parameters.value("threshold");
    m_pRatio = parameters.value("ratio");
    m_pKnee = parameters.value("knee");
    m_pAttack = parameters.value("attack");
    m_pRelease = parameters.value("release");
    m_pLevel = parameters.value("level");
    m_pAutoMakeUp = parameters.value("automakeup");
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
        applyAutoMakeUp(pState, pInput, pOutput, numSamples);
    }

    // Output gain
    CSAMPLE gain = static_cast<CSAMPLE>(db2ratio(m_pLevel->value()));
    SampleUtil::applyGain(pOutput, gain, numSamples);
}

void CompressorEffect::applyAutoMakeUp(CompressorGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const SINT& numSamples) {
    CSAMPLE rmsInput = SampleUtil::rms(pInput, numSamples);
    if (rmsInput > CSAMPLE_ZERO) {
        CSAMPLE_GAIN makeUpGainState = pState->previousMakeUpGain;

        CSAMPLE rmsOutput = SampleUtil::rms(pOutput, numSamples);
        CSAMPLE_GAIN makeUp = rmsInput / rmsOutput;

        // smoothing
        makeUpGainState = kMakeUpAttackCoeff * makeUp + (1 - kMakeUpAttackCoeff) * makeUpGainState;

        pState->previousMakeUpGain = makeUpGainState;
        SampleUtil::applyGain(pOutput, makeUpGainState, numSamples);
    }
}

void CompressorEffect::applyCompression(CompressorGroupState* pState,
        const mixxx::EngineParameters& engineParameters,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput) {
    double thresholdParam = m_pThreshold->value();
    double ratioParam = m_pRatio->value();
    double kneeParam = m_pKnee->value();
    double kneeHalf = kneeParam / 2.0f;

    double attackParamMs = m_pAttack->value();
    double attackCoeff = pState->previousAttackCoeff;
    if (attackParamMs != pState->previousAttackParamMs) {
        attackCoeff = calculateBallistics(attackParamMs, engineParameters);
        pState->previousAttackParamMs = attackParamMs;
        pState->previousAttackCoeff = attackCoeff;
    }

    double releaseParamMs = m_pRelease->value();
    double releaseCoeff = pState->previousReleaseCoeff;
    if (releaseParamMs != pState->previousReleaseParamMs) {
        releaseCoeff = calculateBallistics(releaseParamMs, engineParameters);
        pState->previousReleaseParamMs = releaseParamMs;
        pState->previousReleaseCoeff = releaseCoeff;
    }

    double stateDB = pState->previousStateDB;
    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    for (SINT i = 0; i < numSamples; i += channelCount) {
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        if (maxSample == CSAMPLE_ZERO) {
            pOutput[i] = CSAMPLE_ZERO;
            pOutput[i + 1] = CSAMPLE_ZERO;
            continue;
        }

        double maxSampleDB = ratio2db(maxSample);
        double overDB = maxSampleDB - thresholdParam;
        if (overDB <= -kneeHalf) {
            overDB = 0.0;
        } else if (overDB > -kneeHalf && overDB <= kneeHalf) {
            overDB = 0.5 * (overDB + kneeHalf) * (overDB + kneeHalf) / kneeParam;
        }
        double compressedDB = overDB * (1.0 / ratioParam - 1.0);

        // attack/release
        if (compressedDB < stateDB) {
            stateDB = compressedDB + attackCoeff * (stateDB - compressedDB);
        } else {
            stateDB = compressedDB + releaseCoeff * (stateDB - compressedDB);
        }

        CSAMPLE gain = static_cast<CSAMPLE>(db2ratio(stateDB));
        pOutput[i] = pInput[i] * gain;
        pOutput[i + 1] = pInput[i + 1] * gain;
    }
    pState->previousStateDB = stateDB;
}
