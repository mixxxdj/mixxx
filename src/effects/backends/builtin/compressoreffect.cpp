#include "effects/backends/builtin/compressoreffect.h"

#include "util/math.h"

namespace {
// Auto make up time is empirically selected parameter, which is good enough for most cases
constexpr double defaultMakeUpAttackMs = 150;
constexpr double defaultAttackMs = 1;
constexpr double defaultReleaseMs = 300;
constexpr CSAMPLE_GAIN defaultThresholdDB = -20;

double calculateBallistics(double paramMs, const mixxx::EngineParameters& engineParameters) {
    return exp(-1000.0 / (paramMs * engineParameters.sampleRate()));
}

CSAMPLE_GAIN calculateMakeUpAttackCoeff(const mixxx::EngineParameters& engineParameters) {
    return static_cast<CSAMPLE>(
            (1 - calculateBallistics(defaultMakeUpAttackMs, engineParameters)) *
            engineParameters.framesPerBuffer());
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

void CompressorGroupState::clear(const mixxx::EngineParameters& engineParameters) {
    stateDB = 0;
    attackCoeff = calculateBallistics(defaultAttackMs, engineParameters);
    releaseCoeff = calculateBallistics(defaultReleaseMs, engineParameters);
    makeUpGainState = CSAMPLE_GAIN_ONE;
    makeUpCoeff = calculateMakeUpAttackCoeff(engineParameters);

    previousAttackParamMs = defaultAttackMs;
    previousReleaseParamMs = defaultReleaseMs;
    previousFramesPerBuffer = engineParameters.framesPerBuffer();
    previousSampleRate = engineParameters.sampleRate();
}

void CompressorGroupState::calculateCoeffsIfChanged(
        const mixxx::EngineParameters& engineParameters,
        double attackParamMs,
        double releaseParamMs) {
    if (engineParameters.sampleRate() != previousSampleRate) {
        attackCoeff = calculateBallistics(attackParamMs, engineParameters);
        previousAttackParamMs = attackParamMs;

        releaseCoeff = calculateBallistics(releaseParamMs, engineParameters);
        previousReleaseParamMs = releaseParamMs;

        makeUpCoeff = calculateMakeUpAttackCoeff(engineParameters);
        previousFramesPerBuffer = engineParameters.framesPerBuffer();

        previousSampleRate = engineParameters.sampleRate();
    } else {
        if (attackParamMs != previousAttackParamMs) {
            attackCoeff = calculateBallistics(attackParamMs, engineParameters);
            previousAttackParamMs = attackParamMs;
        }

        if (releaseParamMs != previousReleaseParamMs) {
            releaseCoeff = calculateBallistics(releaseParamMs, engineParameters);
            previousReleaseParamMs = releaseParamMs;
        }

        if (engineParameters.framesPerBuffer() != previousFramesPerBuffer) {
            makeUpCoeff = calculateMakeUpAttackCoeff(engineParameters);
            previousFramesPerBuffer = engineParameters.framesPerBuffer();
        }
    }
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

    if (enableState == EffectEnableState::Enabling) {
        pState->clear(engineParameters);
    } else {
        pState->calculateCoeffsIfChanged(engineParameters, m_pAttack->value(), m_pRelease->value());
    }

    SINT numSamples = engineParameters.samplesPerBuffer();

    // Compression
    CSAMPLE* pGainBuffer = applyCompression(pState, engineParameters, pInput, pOutput);

    // Auto make up
    if (m_pAutoMakeUp->toInt() == static_cast<int>(AutoMakeUp::AutoMakeUpOn)) {
        applyAutoMakeUp(pState, numSamples, pGainBuffer);
    }

    // Output gain
    CSAMPLE gain = static_cast<CSAMPLE>(db2ratio(m_pLevel->value()));

    // Finally applying all gains
    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); i++) {
        pOutput[i] = pInput[i] * pGainBuffer[i] * gain;
    }
}

void CompressorEffect::applyAutoMakeUp(
        CompressorGroupState* pState, SINT numSamples, CSAMPLE* pGainBuffer) {
    CSAMPLE rmsGain = SampleUtil::rms(pGainBuffer, numSamples);

    CSAMPLE_GAIN makeUpGainState = pState->makeUpGainState;

    // smoothing
    makeUpGainState = pState->makeUpCoeff * rmsGain + (1 - pState->makeUpCoeff) * makeUpGainState;

    SampleUtil::applyRampingGain(pGainBuffer,
            1 / pState->makeUpGainState,
            1 / makeUpGainState,
            numSamples);
    pState->makeUpGainState = makeUpGainState;
}

CSAMPLE* CompressorEffect::applyCompression(CompressorGroupState* pState,
        const mixxx::EngineParameters& engineParameters,
        const CSAMPLE* pInput,
        CSAMPLE* pGainBuffer) {
    double thresholdParam = m_pThreshold->value();
    double ratioParam = m_pRatio->value();
    double kneeParam = m_pKnee->value();
    double kneeHalf = kneeParam / 2.0f;

    double stateDB = pState->stateDB;
    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    for (SINT i = 0; i < numSamples; i += channelCount) {
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        if (maxSample == CSAMPLE_ZERO) {
            pGainBuffer[i] = CSAMPLE_ONE;
            pGainBuffer[i + 1] = CSAMPLE_ONE;
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
            stateDB = compressedDB + pState->attackCoeff * (stateDB - compressedDB);
        } else {
            stateDB = compressedDB + pState->releaseCoeff * (stateDB - compressedDB);
        }

        CSAMPLE gain = static_cast<CSAMPLE>(db2ratio(stateDB));
        pGainBuffer[i] = gain;
        pGainBuffer[i + 1] = gain;
    }
    pState->stateDB = stateDB;
    return pGainBuffer;
}
