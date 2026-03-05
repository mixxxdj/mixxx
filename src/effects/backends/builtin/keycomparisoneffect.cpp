#include "effects/backends/builtin/keycomparisoneffect.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>

#include "audio/types.h"
#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectmanifestparameter.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/types.h"

namespace {

// Semitone offsets from A4 (440 Hz) for each chromatic key.
// Index 9 (A) has offset 0 — the sample is synthesised at A4.
// Index: 0=C  1=C#  2=D  3=D#  4=E  5=F  6=F#  7=G  8=G#  9=A  10=A#  11=B
constexpr std::array<int, 12> kKeySemitoneOffset = {
        -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2};

// Mixes a pitched mono source into a stereo-interleaved output buffer by
// stepping through the source at pitchRatio samples per output frame.
// Linear interpolation between adjacent source samples keeps the output
// smooth at non-integer step positions.
// Returns the number of source frames consumed so the caller can resume
// from the correct position in the next buffer.
std::size_t playPitchedMonoIntoStereo(
        std::span<const CSAMPLE> monoSource,
        std::span<CSAMPLE> stereoOutput,
        CSAMPLE_GAIN gain,
        double pitchRatio) {
    const std::size_t outputFrames =
            stereoOutput.size() / mixxx::kEngineChannelOutputCount;

    std::size_t framesConsumed = 0;
    for (std::size_t i = 0; i < outputFrames; ++i) {
        const double srcPos = static_cast<double>(i) * pitchRatio;
        const auto srcIdx = static_cast<std::size_t>(srcPos);

        if (srcIdx >= monoSource.size()) {
            break;
        }

        CSAMPLE s;
        if (srcIdx + 1 < monoSource.size()) {
            const double frac = srcPos - static_cast<double>(srcIdx);
            s = static_cast<CSAMPLE>(
                    monoSource[srcIdx] * (1.0 - frac) +
                    monoSource[srcIdx + 1] * frac);
        } else {
            s = monoSource[srcIdx];
        }

        const std::size_t out = i * mixxx::kEngineChannelOutputCount;
        stereoOutput[out] += s * gain;
        stereoOutput[out + 1] += s * gain;

        framesConsumed = srcIdx + 1;
    }
    return framesConsumed;
}

template<class T>
std::span<T> subspanClamped(
        std::span<T> in, typename std::span<T>::size_type offset) {
    return in.subspan(std::min(offset, in.size()));
}

// Returns the subspan of output starting at the beat position.
// Mirrors metronomeeffect's syncedClickOutput exactly.
std::span<CSAMPLE> syncedNoteOutput(
        double beatFractionBufferEnd,
        std::optional<GroupFeatureBeatLength> beatLengthAndScratch,
        const mixxx::EngineParameters& engineParameters,
        std::span<CSAMPLE> output) {
    if (!beatLengthAndScratch.has_value() ||
            beatLengthAndScratch->scratch_rate == 0.0) {
        return {};
    }
    double beatLength = beatLengthAndScratch->seconds *
            engineParameters.sampleRate() / beatLengthAndScratch->scratch_rate;

    const bool needsPreviousBeat = beatLength < 0;
    double beatToBufferEndFrames = std::abs(beatLength) *
            (needsPreviousBeat ? (1 - beatFractionBufferEnd)
                               : beatFractionBufferEnd);
    std::size_t beatToBufferEndSamples =
            static_cast<std::size_t>(beatToBufferEndFrames) *
            mixxx::kEngineChannelOutputCount;

    if (beatToBufferEndSamples <= output.size()) {
        return output.last(beatToBufferEndSamples);
    }
    return {};
}

// Returns where in the output buffer the next note starts using the internal
// BPM counter. Mirrors metronomeeffect's unsyncedClickOutput.
std::span<CSAMPLE> unsyncedNoteOutput(
        mixxx::audio::SampleRate framesPerSecond,
        std::size_t framesSinceLastNote,
        double bpm,
        double periodMultiplier,
        std::span<CSAMPLE> output) {
    const std::size_t period = static_cast<std::size_t>(
            framesPerSecond * 60.0 / bpm * periodMultiplier);
    if (period == 0) {
        return {};
    }
    const std::size_t offset = framesSinceLastNote % period;
    return subspanClamped(output, offset * mixxx::kEngineChannelOutputCount);
}

// Returns the number of beats between each note onset.
// 1 fires on every beat, 3 suits 3/4 time, 4 suits 4/4 time, and so on.
double periodMultiplierFromMeasure(int measure) {
    return static_cast<double>(std::max(1, measure));
}

} // namespace

void KeyComparisonGroupState::audioParametersChanged(
        const mixxx::EngineParameters& engineParameters) {
    m_sampleRate = engineParameters.sampleRate();
    pianoSample = generatePianoSample(m_sampleRate);
}

// static
QString KeyComparisonEffect::getId() {
    return QStringLiteral("org.mixxx.effects.keycomparison");
}

// static
EffectManifestPointer KeyComparisonEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Key Comparison"));
    pManifest->setAuthor(QObject::tr("The Mixxx Team"));
    pManifest->setVersion(QStringLiteral("1.0"));
    pManifest->setDescription(QObject::tr(
            "Plays a piano note at a configurable beat interval so you "
            "can match it by ear to identify or verify the musical key."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(24.0 / 27.0);

    EffectManifestParameterPointer key = pManifest->addParameter();
    key->setId(QStringLiteral("key"));
    key->setName(QObject::tr("Key"));
    key->setShortName(QObject::tr("Key"));
    key->setDescription(QObject::tr(
            "Musical key of the piano note (12 chromatic steps).\n"
            "0=C  1=C\u266f/D\u266d  2=D  3=D\u266f/E\u266d  "
            "4=E  5=F\n"
            "6=F\u266f/G\u266d  7=G  8=G\u266f/A\u266d  "
            "9=A  10=A\u266f/B\u266d  11=B"));
    key->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    key->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    key->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    key->setRange(0.0, 9.0, 11.0);

    EffectManifestParameterPointer tuning = pManifest->addParameter();
    tuning->setId(QStringLiteral("tuning"));
    tuning->setName(QObject::tr("Tuning"));
    tuning->setShortName(QObject::tr("Tune"));
    tuning->setDescription(QObject::tr(
            "Reference pitch of A4 in Hz. Adjust this when the track was\n"
            "recorded to a non-standard tuning such as 432 Hz, 442 Hz,\n"
            "or 415 Hz (Baroque pitch)."));
    tuning->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    tuning->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    tuning->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    tuning->setNeutralPointOnScale(25.0 / 51.0);
    tuning->setRange(415.0, 440.0, 466.0);

    EffectManifestParameterPointer bpm = pManifest->addParameter();
    bpm->setId(QStringLiteral("bpm"));
    bpm->setName(QObject::tr("BPM"));
    bpm->setShortName(QObject::tr("BPM"));
    bpm->setDescription(QObject::tr(
            "Note repeat rate when Sync is off."));
    bpm->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bpm->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bpm->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    bpm->setRange(60.0, 120.0, 200.0);

    EffectManifestParameterPointer measure = pManifest->addParameter();
    measure->setId(QStringLiteral("measure"));
    measure->setName(QObject::tr("Measure"));
    measure->setShortName(QObject::tr("Meas"));
    measure->setDescription(QObject::tr(
            "How many beats between each note.\n"
            "1 = every beat  |  3 = 3/4 time  |  "
            "4 = 4/4 time  |  5 = 5/4 time"));
    measure->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    measure->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    measure->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    measure->setRange(1.0, 4.0, 8.0);

    EffectManifestParameterPointer sync = pManifest->addParameter();
    sync->setId(QStringLiteral("sync"));
    sync->setName(QObject::tr("Sync"));
    sync->setShortName(QObject::tr("Sync"));
    sync->setDescription(QObject::tr(
            "Lock note timing to the detected beat grid (recommended).\n"
            "Disable to set a manual rate with the BPM knob."));
    sync->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    sync->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    sync->setRange(0.0, 1.0, 1.0);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId(QStringLiteral("gain"));
    gain->setName(QObject::tr("Gain"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr(
            "Volume of the piano note."));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    gain->setNeutralPointOnScale(24.0 / 27.0);
    gain->setRange(-24.0, -5.0, 3.0);

    return pManifest;
}

void KeyComparisonEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pKeyParameter = parameters.value(QStringLiteral("key"));
    m_pTuningParameter = parameters.value(QStringLiteral("tuning"));
    m_pBpmParameter = parameters.value(QStringLiteral("bpm"));
    m_pMeasureParameter = parameters.value(QStringLiteral("measure"));
    m_pSyncParameter = parameters.value(QStringLiteral("sync"));
    m_pGainParameter = parameters.value(QStringLiteral("gain"));
}

void KeyComparisonEffect::processChannel(
        KeyComparisonGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    if (enableState == EffectEnableState::Disabled) {
        return;
    }

    // audioParametersChanged may be called with a stale rate during init.
    // Regenerate the sample here on the first buffer at the real engine rate.
    if (engineParameters.sampleRate() != pGroupState->m_sampleRate) {
        pGroupState->audioParametersChanged(engineParameters);
    }

    if (pOutput != pInput) {
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
    }

    const std::span<CSAMPLE> output(pOutput, engineParameters.samplesPerBuffer());
    const std::span<const CSAMPLE> pianoSample(pGroupState->pianoSample);

    const bool shouldSync = m_pSyncParameter->toBool();
    const bool hasBeatInfo = groupFeatures.beat_length.has_value() &&
            groupFeatures.beat_fraction_buffer_end.has_value();

    if (enableState == EffectEnableState::Enabling) {
        if (shouldSync && hasBeatInfo) {
            pGroupState->framesSinceLastNote = pianoSample.size();
        } else {
            pGroupState->framesSinceLastNote = 0;
        }
        pGroupState->m_beatCount = 0;
    }

    const int keyIndex = std::clamp(
            static_cast<int>(std::round(m_pKeyParameter->value())),
            0,
            static_cast<int>(kKeySemitoneOffset.size()) - 1);
    // pitchRatio = 2^(semitones/12) * (tuningHz / 440)
    const double pitchRatio =
            std::pow(2.0, kKeySemitoneOffset[keyIndex] / 12.0) *
            (m_pTuningParameter->value() / 440.0);

    const CSAMPLE_GAIN gain =
            db2ratio(static_cast<float>(m_pGainParameter->value()));

    const int measure = std::clamp(
            static_cast<int>(std::round(m_pMeasureParameter->value())),
            1,
            8);
    const double periodMultiplier = periodMultiplierFromMeasure(measure);

    // Continue the note tail from the previous buffer.
    const auto srcOffset = static_cast<std::size_t>(
            static_cast<double>(pGroupState->framesSinceLastNote) *
            pitchRatio);
    playPitchedMonoIntoStereo(
            subspanClamped(pianoSample, srcOffset), output, gain, pitchRatio);
    pGroupState->framesSinceLastNote += engineParameters.framesPerBuffer();

    const std::span<CSAMPLE> noteStart = shouldSync && hasBeatInfo
            ? syncedNoteOutput(
                      *groupFeatures.beat_fraction_buffer_end,
                      groupFeatures.beat_length,
                      engineParameters,
                      output)
            : unsyncedNoteOutput(
                      engineParameters.sampleRate(),
                      pGroupState->framesSinceLastNote,
                      m_pBpmParameter->value(),
                      periodMultiplier,
                      output);

    if (noteStart.empty()) {
        return;
    }

    // In sync mode, count beats and only fire every `measure` beats.
    if (shouldSync && hasBeatInfo) {
        pGroupState->m_beatCount++;
        if (pGroupState->m_beatCount < measure) {
            return;
        }
        pGroupState->m_beatCount = 0;
    }

    pGroupState->framesSinceLastNote =
            playPitchedMonoIntoStereo(pianoSample, noteStart, gain, pitchRatio);
}
