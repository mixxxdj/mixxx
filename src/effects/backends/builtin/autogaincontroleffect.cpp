#include "effects/backends/builtin/autogaincontroleffect.h"

#include "util/math.h"

namespace {
constexpr double defaultAttackMs = 1;
constexpr double defaultReleaseMs = 250;
constexpr double defaultThresholdDB = -40;
constexpr double defaultTargetDB = -5;
constexpr double defaultGainDB = 20;

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
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription("Auto Gain Control (AGC) effect");
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
    threshold->setRange(-70, defaultThresholdDB, 0);

    EffectManifestParameterPointer target = pManifest->addParameter();
    target->setId("target");
    target->setName(QObject::tr("Target (dBFS)"));
    target->setShortName(QObject::tr("Target"));
    target->setDescription(
            QObject::tr("The Target knob adjusts the desired target level of the output signal"));
    target->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    target->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    target->setNeutralPointOnScale(0);
    target->setRange(-20, defaultTargetDB, 10);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId("gain");
    gain->setName(QObject::tr("Gain (dB)"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(
            QObject::tr("The Gain knob adjusts the maximum amount of gain that "
                        "the effect will apply"));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setRange(1, defaultGainDB, 40);

    EffectManifestParameterPointer attack = pManifest->addParameter();
    attack->setId("attack");
    attack->setName(QObject::tr("Attack (ms)"));
    attack->setShortName(QObject::tr("Attack"));
    attack->setDescription(QObject::tr(
            "The Attack knob sets the time that determines how fast the "
            "auto gain \nwill set in once the signal exceeds the threshold"));
    attack->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    attack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    attack->setRange(0, defaultAttackMs, 250);

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
    release->setRange(0, defaultReleaseMs, 1500);

    return pManifest;
}

void AutoGainControlGroupState::clear(const mixxx::EngineParameters& engineParameters) {
    state = CSAMPLE_ONE;
    attackCoeff = calculateBallistics(defaultAttackMs, engineParameters);
    releaseCoeff = calculateBallistics(defaultReleaseMs, engineParameters);

    previousAttackParamMs = defaultAttackMs;
    previousReleaseParamMs = defaultReleaseMs;
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
    CSAMPLE threshold = db2ratio(m_pThreshold->value());
    CSAMPLE target = db2ratio(m_pTarget->value());
    CSAMPLE_GAIN maxGain = db2ratio(m_pGain->value());
    double kneeDB = 5.0; // TODO
    double thresholdDB = m_pThreshold->value();
    double targetLevelDB = m_pTarget->value();
    double maxGainDB = m_pGain->value();

    CSAMPLE state = pState->state;

    SINT numSamples = engineParameters.samplesPerBuffer();
    int channelCount = engineParameters.channelCount();
    for (SINT i = 0; i < numSamples; i += channelCount) {
        CSAMPLE maxSample = std::max(fabs(pInput[i]), fabs(pInput[i + 1]));
        if (maxSample == CSAMPLE_ZERO) {
            pOutput[i] = CSAMPLE_ZERO;
            pOutput[i + 1] = CSAMPLE_ZERO;
            continue;
        }

        // TODO поменять местами определение attack/release и вычисление gain
        // (как в изначальном нашем варианте из файлика)

        // Конвертируем текущее значение в dB
        double inputLevelDB = ratio2db(maxSample);

        // Рассчитываем необходимый гейн
        double desiredGainDB = 0.0;

        double upperKnee = thresholdDB + 0.5 * kneeDB;
        double lowerKnee = thresholdDB - 0.5 * kneeDB;

        if (inputLevelDB > upperKnee) {
            // Если уровень выше верхней границы "Knee", рассчитываем гейн как обычно
            desiredGainDB = targetLevelDB - inputLevelDB;
        } else if (inputLevelDB < lowerKnee) {
            // Если уровень ниже нижней границы "Knee", гейн равен 0
            desiredGainDB = 0.0;
        } else {
            // Если сигнал находится внутри зоны "Knee", применяем плавный переход гейна
            double kneePosition = (inputLevelDB - lowerKnee) / kneeDB;
            desiredGainDB = (targetLevelDB - upperKnee) * kneePosition;
        }

        // Применяем ограничение на максимальный гейн
        desiredGainDB = std::min(desiredGainDB, maxGainDB);

        // Переводим нужный гейн в амплитудный коэффициент
        CSAMPLE_GAIN gain = db2ratio(desiredGainDB);
        // if (maxSample > threshold) {
        //     gain = target / maxSample;
        //     if (gain > maxGain) {
        //         gain = maxGain;
        //     }
        // } else {
        //     gain = CSAMPLE_GAIN_ONE;
        // }

        // TODO: maxGain!
        // TODO: threshold doesn't work if signal is lower!
        // CSAMPLE_GAIN gain = target / state;
        // if (gain > maxGain) {
        //     gain = maxGain;
        // }
        //  == another:
        // if (state > threshold) {
        //     gain = target / state;
        // }
        // else {
        //     gain = target / threshold;
        // }

        if (gain < state) {
            state = pState->attackCoeff * (state - gain) + gain;
        } else {
            state = pState->releaseCoeff * (state - gain) + gain;
        }

        pOutput[i] = pInput[i] * state;
        pOutput[i + 1] = pInput[i + 1] * state;
    }
    pState->state = state;
}
