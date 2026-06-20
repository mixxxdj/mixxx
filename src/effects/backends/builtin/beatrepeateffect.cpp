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
    pInterval->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
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
    pDecay->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
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
    const SINT numSamples = engineParameters.samplesPerBuffer();
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
    double bpm = 120.0;
    double beatSamples = (60.0 / bpm) * sampleRate * chCount;
    int loopLen = std::max(chCount, static_cast<int>(smoothInterval * beatSamples));

    if (loopLen > pState->delay_buf.size()) {
        loopLen = pState->delay_buf.size();
    }

    // State machine: recording -> repeating
    if (enable > 0.5) {
        if (pState->is_recording) {
            // Record into buffer
            SINT recorded = 0;
            for (SINT i = 0; i < numSamples; ++i) {
                pState->delay_buf[pState->write_position] = pInput[i];
                pOutput[i] = pInput[i]; // Pass through while recording
                pState->write_position = (pState->write_position + 1) % pState->delay_buf.size();
                recorded++;
                if ((int)recorded >= loopLen) {
                    // Finished recording, start repeating
                    pState->is_recording = false;
                    pState->loop_start = 0;
                    pState->loop_length = loopLen;
                    pState->read_position = 0.0;
                    pState->repeat_count = 0;
                    pState->fade_state = 1.0f;
                    // Process remaining samples in repeat mode
                    for (SINT j = i + 1; j < numSamples; ++j) {
                        CSAMPLE sample = processRepeatSample(pState, loopLen, decay, gate);
                        pOutput[j] = sample;
                    }
                    break;
                }
            }
        } else {
            // Repeat mode
            for (SINT i = 0; i < numSamples; ++i) {
                CSAMPLE sample = processRepeatSample(pState, loopLen, decay, gate);
                pOutput[i] = sample;
            }
        }
    } else {
        // Disabled: pass through and reset
        for (SINT i = 0; i < numSamples; ++i) {
            pOutput[i] = pInput[i];
        }
        pState->is_recording = true;
        pState->write_position = 0;
        pState->repeat_count = 0;
        pState->fade_state = 1.0f;
    }
}

CSAMPLE BeatRepeatEffect::processRepeatSample(BeatRepeatGroupState* pState,
        int loopLen,
        CSAMPLE_GAIN decay,
        CSAMPLE_GAIN gate) {
    CSAMPLE_GAIN volume = pState->fade_state;
    CSAMPLE_GAIN decayPerSample = decay * 0.001f;

    // Read from loop with optional pitch shift (linear interpolation)
    int readPos = pState->loop_start + static_cast<int>(pState->read_position);
    int readPos2 = readPos + 1;
    double frac = pState->read_position - static_cast<int>(pState->read_position);

    // Wrap positions
    if (readPos >= pState->delay_buf.size()) {
        readPos = readPos % pState->delay_buf.size();
    }
    if (readPos2 >= pState->delay_buf.size()) {
        readPos2 = readPos2 % pState->delay_buf.size();
    }

    CSAMPLE sample = static_cast<CSAMPLE>(
            pState->delay_buf[readPos] * (1.0 - frac) +
            pState->delay_buf[readPos2] * frac);
    sample *= volume;

    // Gate effect: mute portions of the loop
    if (gate > 0.5 && loopLen > 4) {
        int halfLoop = loopLen / 2;
        if (halfLoop > 0) {
            int gatePos = static_cast<int>(pState->read_position) % halfLoop;
            int quarterLoop = halfLoop / 2;
            if (quarterLoop > 0 && gatePos >= quarterLoop) {
                sample = 0.0f;
            }
        }
    }

    // Advance read position with pitch shift
    double pitchMultiplier = std::pow(2.0, static_cast<double>(pState->prev_pitch));
    if (pitchMultiplier < 0.25) {
        pitchMultiplier = 0.25;
    }
    if (pitchMultiplier > 4.0) {
        pitchMultiplier = 4.0;
    }

    pState->read_position += pitchMultiplier;

    if (pState->read_position >= pState->loop_length) {
        pState->read_position = 0.0;
        pState->repeat_count++;
        volume -= decayPerSample * pState->loop_length;
        if (volume < 0.0f) {
            volume = 0.0f;
        }
        pState->fade_state = volume;
    }

    return sample;
}
