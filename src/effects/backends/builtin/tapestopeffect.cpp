#include "effects/backends/builtin/tapestopeffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

namespace {
constexpr double kMinSpeed = 0.01;
} // anonymous namespace

QString TapeStopEffect::getId() {
    return QStringLiteral("org.mixxx.effects.tapestop");
}

EffectManifestPointer TapeStopEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Tape Stop"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Simulates a tape machine slowing down to a stop. "
            "Creates a dramatic pitch-down effect. Popular in DJ transitions."));

    auto pEnable = pManifest->addParameter();
    pEnable->setId("enable");
    pEnable->setName(QObject::tr("Enable"));
    pEnable->setDescription(QObject::tr("Toggle tape stop on/off"));
    pEnable->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    pEnable->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pEnable->setRange(0.0, 0.0, 1.0);

    auto pSpeed = pManifest->addParameter();
    pSpeed->setId("speed");
    pSpeed->setName(QObject::tr("Speed"));
    pSpeed->setDescription(QObject::tr("Initial playback speed"));
    pSpeed->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pSpeed->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pSpeed->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pSpeed->setRange(0.1, 1.0, 2.0);

    auto pDuration = pManifest->addParameter();
    pDuration->setId("duration");
    pDuration->setName(QObject::tr("Duration"));
    pDuration->setDescription(QObject::tr("Stop duration in seconds"));
    pDuration->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDuration->setUnitsHint(EffectManifestParameter::UnitsHint::Seconds);
    pDuration->setRange(0.1, 1.0, 4.0);

    return pManifest;
}

void TapeStopEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pEnableParameter = parameters.value("enable");
    m_pSpeedParameter = parameters.value("speed");
    m_pDurationParameter = parameters.value("duration");
}

void TapeStopEffect::processChannel(
        TapeStopGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const int sampleRate = engineParameters.sampleRate();
    const SINT numSamples = engineParameters.samplesPerBuffer();
    const int chCount = engineParameters.channelCount();

    const CSAMPLE_GAIN enable = static_cast<CSAMPLE_GAIN>(m_pEnableParameter->value());
    const CSAMPLE_GAIN speed = static_cast<CSAMPLE_GAIN>(m_pSpeedParameter->value());
    const CSAMPLE_GAIN duration = static_cast<CSAMPLE_GAIN>(m_pDurationParameter->value());

    const double smoothEnable = pState->prev_enable + 0.02 * (enable - pState->prev_enable);
    const double smoothSpeed = pState->prev_speed + 0.02 * (speed - pState->prev_speed);

    pState->prev_enable = static_cast<CSAMPLE_GAIN>(smoothEnable);
    pState->prev_speed = static_cast<CSAMPLE_GAIN>(smoothSpeed);

    if (enable > 0.5) {
        if (!pState->is_stopping) {
            // Start the tape stop
            pState->is_stopping = true;
            pState->speed = smoothSpeed;
            pState->stop_start_pos = pState->write_position;
        }

        // Calculate deceleration rate per sample (total samples = duration * sampleRate * chCount)
        const double totalSamples = duration * sampleRate * chCount;
        const double decelRate = totalSamples > 0.0
                ? (pState->speed - kMinSpeed) / totalSamples
                : 0.0;

        for (SINT i = 0; i < numSamples; ++i) {
            // Decelerate
            pState->speed -= decelRate;
            if (pState->speed < kMinSpeed) {
                pState->speed = kMinSpeed;
            }

            // Read from buffer at current speed
            int readPos = static_cast<int>(pState->read_position);
            if (readPos >= pState->delay_buf.size()) {
                readPos = readPos % pState->delay_buf.size();
            }
            if (readPos < 0) {
                readPos = 0;
            }

            // Linear interpolation for smooth pitch shifting
            CSAMPLE sample1 = pState->delay_buf[readPos];
            CSAMPLE sample2 = pState->delay_buf[(readPos + 1) % pState->delay_buf.size()];
            double frac = pState->read_position - readPos;
            CSAMPLE interpolated = static_cast<CSAMPLE>(sample1 + (sample2 - sample1) * frac);

            pOutput[i] = interpolated;

            // Advance read position
            pState->read_position += pState->speed;
            if (pState->read_position >= pState->delay_buf.size()) {
                pState->read_position -= pState->delay_buf.size();
            }

            // Write current input to buffer (for when we loop)
            pState->delay_buf[pState->write_position] = pInput[i];
            pState->write_position = (pState->write_position + 1) % pState->delay_buf.size();
        }
    } else {
        // Disabled: pass through
        for (SINT i = 0; i < numSamples; ++i) {
            pOutput[i] = pInput[i];
            pState->delay_buf[pState->write_position] = pInput[i];
            pState->write_position = (pState->write_position + 1) % pState->delay_buf.size();
        }
        pState->is_stopping = false;
        pState->speed = 1.0;
        pState->read_position = pState->write_position;
    }
}
