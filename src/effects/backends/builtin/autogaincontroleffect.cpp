#include "effects/backends/builtin/autogaincontroleffect.h"

#include "util/math.h"

namespace {
using namespace std::chrono_literals;
constexpr std::chrono::milliseconds kDefaultAttack = 1ms;
constexpr std::chrono::milliseconds kDefaultRelease = 500ms;
constexpr double kDefaultThresholdDB = -40;
constexpr double kDefaultTargetDB = -5;
constexpr double kDefaultGainDB = 20;
constexpr double kDefaultKneeDB = 10;

double calculateBallistics(double paramMs, const mixxx::EngineParameters& engineParameters) {
    return exp(-1000.0 / (paramMs * engineParameters.sampleRate()));
}
} // anonymous namespace

// static
QString AutoGainControlEffect::getId() {
    return "org.mixxx.effects.autogaincontrol";
}

// static
EffectManifestPointer AutoGainControlEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Auto Gain Control"));
    pManifest->setShortName(QObject::tr("AGC"));
    pManifest->setAuthor(QObject::tr("The Mixxx Team"));
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Auto Gain Control (AGC) automatically adjusts the gain of an "
            "audio signal to maintain a consistent output level."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer threshold = pManifest->addParameter();
    threshold->setId("threshold");
    threshold->setName(QObject::tr("Threshold (dBFS)"));
    threshold->setShortName(QObject::tr("Threshold"));
    threshold->setDescription(
            QObject::tr("The Threshold knob adjusts the level above which the "
                        "effect starts enhancing the input signal"));
    threshold->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    threshold->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    threshold->setNeutralPointOnScale(0);
    threshold->setRange(-70, kDefaultThresholdDB, 0);

    EffectManifestParameterPointer target = pManifest->addParameter();
    target->setId("target");
    target->setName(QObject::tr("Target (dBFS)"));
    target->setShortName(QObject::tr("Target"));
    target->setDescription(
            QObject::tr("The Target knob adjusts the desired target level of the output signal"));
    target->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    target->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    target->setNeutralPointOnScale(0);
    target->setRange(-20, kDefaultTargetDB, 10);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain (dB)"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(
            QObject::tr("The Gain knob adjusts the maximum amount of gain that "
                        "the effect will apply"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setRange(1, kDefaultGainDB, 40);

    EffectManifestParameterPointer knee = pManifest->addParameter();
    knee->setId("knee");
    knee->setName(QObject::tr("Knee (dB)"));
    knee->setShortName(QObject::tr("Knee"));
    knee->setDescription(QObject::tr(
            "The Knee knob defines the range around the Threshold where gain "
            "changes are applied gradually,\nensuring smooth transitions and "
            "avoiding abrupt level shifts."));
    knee->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    knee->setUnitsHint(EffectManifestParameter::UnitsHint::Coefficient);
    knee->setNeutralPointOnScale(0);
    knee->setRange(0.0, kDefaultKneeDB, 24);

    EffectManifestParameterPointer attack = pManifest->addParameter();
    attack->setId("attack");
    attack->setName(QObject::tr("Attack (ms)"));
    attack->setShortName(QObject::tr("Attack"));
    attack->setDescription(QObject::tr(
            "The Attack knob sets the time that determines how fast the "
            "auto gain \nwill set in once the signal exceeds the threshold"));
    attack->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    attack->setRange(0, kDefaultAttack.count(), 250);

    EffectManifestParameterPointer release = pManifest->addParameter();
    release->setId("release");
    release->setName(QObject::tr("Release (ms)"));
    release->setShortName(QObject::tr("Release"));
    release->setDescription(
            QObject::tr("The Release knob sets the time that determines how "
                        "fast the auto gain will recover from the gain\n"
                        "adjustment once the signal falls under the threshold. "
                        "Depending on the input signal, short release times\n"
                        "may introduce a 'pumping' effect and/or distortion."));
    release->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    release->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    release->setRange(0, kDefaultRelease.count(), 1500);

    return pManifest;
}

void AutoGainControlGroupState::clear(const mixxx::EngineParameters& engineParameters) {
    state = CSAMPLE_ONE;
    attackCoeff = calculateBallistics(kDefaultAttack.count(), engineParameters);
    releaseCoeff = calculateBallistics(kDefaultRelease.count(), engineParameters);

    previousAttackParamMs = kDefaultAttack.count();
    previousReleaseParamMs = kDefaultRelease.count();
    previousSampleRate = engineParameters.sampleRate();
}

void AutoGainControlGroupState::calculateCoeffsIfChanged(
        const mixxx::EngineParameters& engineParameters,
        double attackParamMs,
        double releaseParamMs) {
    if (engineParameters.sampleRate() != previousSampleRate) {
        attackCoeff = calculateBallistics(attackParamMs, engineParameters);
        previousAttackParamMs = attackParamMs;

        releaseCoeff = calculateBallistics(releaseParamMs, engineParameters);
        previousReleaseParamMs = releaseParamMs;

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
    }
}

void AutoGainControlEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pThreshold = parameters.value("threshold");
    m_pTarget = parameters.value("target");
    m_pGain = parameters.value("gain");
    m_pKnee = parameters.value("knee");
    m_pAttack = parameters.value("attack");
    m_pRelease = parameters.value("release");
}

void AutoGainControlEffect::processChannel(
        AutoGainControlGroupState* pState,
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

    applyAutoGainControl(pState, engineParameters, pInput, pOutput);
}

void AutoGainControlEffect::applyAutoGainControl(AutoGainControlGroupState* pState,
        const mixxx::EngineParameters& engineParameters,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput) {
    // Get user-defined parameters
    double thresholdDB = m_pThreshold->value();
    double targetLevelDB = m_pTarget->value();
    double maxGainDB = m_pGain->value();
    double kneeDB = m_pKnee->value();

    // Define the upper and lower boundaries of the knee region
    double upperKneeDB = thresholdDB + 0.5 * kneeDB;
    double lowerKneeDB = thresholdDB - 0.5 * kneeDB;

    // Initialize the envelope state
    double state = pState->state;

    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    for (SINT i = 0; i < numSamples; i += channelCount) {
        // Detect peak level across stereo channels
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));

        // If the input is silent, output silence
        if (maxSample == CSAMPLE_ZERO) {
            pOutput[i] = CSAMPLE_ZERO;
            pOutput[i + 1] = CSAMPLE_ZERO;
            continue;
        }

        // Smooth the level detector using attack/release envelope
        if (maxSample > state) {
            state = pState->attackCoeff * state + (1 - pState->attackCoeff) * maxSample;
        } else {
            state = pState->releaseCoeff * state + (1 - pState->releaseCoeff) * maxSample;
        }

        // Convert current signal level to decibels
        double inputLevelDB = ratio2db(state);

        // Determine the appropriate gain based on the input level
        double desiredGainDB;
        if (inputLevelDB > upperKneeDB) {
            // Above the knee range: apply full gain reduction
            desiredGainDB = targetLevelDB - inputLevelDB;
        } else if (inputLevelDB < lowerKneeDB) {
            // Below the knee range: no gain applied
            desiredGainDB = 0.0;
        } else {
            // Within the knee: interpolate gain smoothly
            double kneePosition = (inputLevelDB - lowerKneeDB) / kneeDB;
            desiredGainDB = (targetLevelDB - upperKneeDB) * kneePosition;
        }

        // Limit the gain to the maximum allowed value
        desiredGainDB = std::min(desiredGainDB, maxGainDB);

        // Convert gain from decibels to linear amplitude ratio
        CSAMPLE_GAIN gain = static_cast<CSAMPLE>(db2ratio(desiredGainDB));

        pOutput[i] = pInput[i] * gain;
        pOutput[i + 1] = pInput[i + 1] * gain;
    }

    // Store the envelope state for the next buffer
    pState->state = state;
}
