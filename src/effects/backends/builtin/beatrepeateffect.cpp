#include "effects/backends/builtin/beatrepeateffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

namespace {
constexpr double kSmoothCoeff = 0.02;
} // anonymous namespace

QString BeatRepeatEffect::getId() {
    return QStringLiteral("org.mixxx.effects.beatrepeat");
}

EffectManifestPointer BeatRepeatEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Beat Repeat"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Beat repeat / stutter effect. Captures a loop and repeats it "
            "at various speeds. Similar to Rekordbox's Beat Repeat."));

    auto pEnable = pManifest->addParameter();
    pEnable->setId("enable");
    pEnable->setName(QObject::tr("Enable"));
    pEnable->setDescription(QObject::tr("Toggle beat repeat on/off"));
    pEnable->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    pEnable->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pEnable->setRange(0.0, 0.0, 1.0);

    auto pInterval = pManifest->addParameter();
    pInterval->setId("interval");
    pInterval->setName(QObject::tr("Interval"));
    pInterval->setDescription(QObject::tr("Loop length in beats"));
    pInterval->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    pInterval->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    pInterval->setRange(0.25, 1.0, 4.0);

    auto pPitch = pManifest->addParameter();
    pPitch->setId("pitch");
    pPitch->setName(QObject::tr("Pitch"));
    pPitch->setDescription(QObject::tr("Pitch shift for repeats (octaves)"));
    pPitch->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pPitch->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pPitch->setRange(-1.0, 0.0, 1.0);

    auto pDecay = pManifest->addParameter();
    pDecay->setId("decay");
    pDecay->setName(QObject::tr("Decay"));
    pDecay->setDescription(QObject::tr("Volume decay per repeat"));
    pDecay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDecay->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pDecay->setRange(0.0, 0.0, 1.0);

    auto pGate = pManifest->addParameter();
    pGate->setId("gate");
    pGate->setName(QObject::tr("Gate"));
    pGate->setDescription(QObject::tr("Gate the repeats (stutter effect)"));
    pGate->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    pGate->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pGate->setRange(0.0, 0.0, 1.0);

    return pManifest;
}

void BeatRepeatEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pEnableParameter = parameters.value("enable");
    m_pIntervalParameter = parameters.value("interval");
    m_pPitchParameter = parameters.value("pitch");
    m_pDecayParameter = parameters.value("decay");
    m_pGateParameter = parameters.value("gate");
}

void BeatRepeatEffect::processChannel(
        BeatRepeatGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const int sampleRate = engineParameters.sampleRate();
    const int numSamples = engineParameters.framesPerBuffer();
    const int chCount = engineParameters.channelCount();

    const CSAMPLE_GAIN enable = static_cast<CSAMPLE_GAIN>(m_pEnableParameter->value());
    const CSAMPLE_GAIN interval = static_cast<CSAMPLE_GAIN>(m_pIntervalParameter->value());
    const CSAMPLE_GAIN pitch = static_cast<CSAMPLE_GAIN>(m_pPitchParameter->value());
    const CSAMPLE_GAIN decay = static_cast<CSAMPLE_GAIN>(m_pDecayParameter->value());
    const CSAMPLE_GAIN gate = static_cast<CSAMPLE_GAIN>(m_pGateParameter->value());

    // Smooth parameters
    double smoothEnable = pState->prev_enable + kSmoothCoeff * (enable - pState->prev_enable);
    double smoothInterval = pState->prev_interval +
            kSmoothCoeff * (interval - pState->prev_interval);
    double smoothPitch = pState->prev_pitch + kSmoothCoeff * (pitch - pState->prev_pitch);

    pState->prev_enable = static_cast<CSAMPLE_GAIN>(smoothEnable);
    pState->prev_interval = static_cast<CSAMPLE_GAIN>(smoothInterval);
    pState->prev_pitch = static_cast<CSAMPLE_GAIN>(smoothPitch);

    // Calculate loop length in samples based on interval (beats)
    // Assume 120 BPM default if no tempo info
    double bpm = 120.0;
    double beatSamples = (60.0 / bpm) * sampleRate;
    int loopLen = std::max(1, static_cast<int>(smoothInterval * beatSamples * chCount));

    if (loopLen > pState->delay_buf.size()) {
        loopLen = pState->delay_buf.size();
    }

    // State machine: recording -> repeating
    if (enable > 0.5) {
        if (pState->is_recording) {
            // Record into buffer
            for (int i = 0; i < numSamples; ++i) {
                pState->delay_buf[pState->write_position] = pInput[i];
                pOutput[i] = pInput[i]; // Pass through while recording
                pState->write_position = (pState->write_position + 1) % pState->delay_buf.size();

                if (pState->write_position == loopLen) {
                    // Finished recording, start repeating
                    pState->is_recording = false;
                    pState->loop_start = 0;
                    pState->loop_length = loopLen;
                    pState->read_position = 0;
                    pState->repeat_count = 0;
                    pState->fade_state = 1.0f;
                    break;
                }
            }
        } else {
            // Repeat mode
            CSAMPLE_GAIN volume = static_cast<CSAMPLE_GAIN>(pState->fade_state);
            CSAMPLE_GAIN decayPerSample = static_cast<CSAMPLE_GAIN>(decay * 0.001f); // Slow decay

            for (int i = 0; i < numSamples; ++i) {
                // Read from loop with optional pitch shift (simple resampling)
                int readPos = pState->loop_start + pState->read_position;
                if (readPos >= pState->delay_buf.size()) {
                    readPos -= pState->delay_buf.size();
                }

                CSAMPLE sample = pState->delay_buf[readPos] * volume;

                // Gate effect: mute every other beat
                if (gate > 0.5) {
                    int gatePos = pState->read_position % (loopLen / 2);
                    if (gatePos >= loopLen / 4) {
                        sample *= 0.0f;
                    }
                }

                pOutput[i] = sample;

                // Advance read position with pitch shift
                double pitchMultiplier = std::pow(2.0, smoothPitch);
                pState->read_position += static_cast<int>(pitchMultiplier);

                if (pState->read_position >= pState->loop_length) {
                    pState->read_position = 0;
                    pState->repeat_count++;
                    volume -= decayPerSample * pState->loop_length;
                    if (volume < 0.0f) {
                        volume = 0.0f;
                    }
                }
            }

            pState->fade_state = volume;
        }
    } else {
        // Disabled: pass through and reset
        for (int i = 0; i < numSamples; ++i) {
            pOutput[i] = pInput[i];
        }
        pState->is_recording = true;
        pState->write_position = 0;
        pState->repeat_count = 0;
        pState->fade_state = 1.0f;
    }
}
