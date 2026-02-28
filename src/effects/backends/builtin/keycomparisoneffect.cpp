#include "effects/backends/builtin/keycomparisoneffect.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>

#include "audio/types.h"
#include "effects/backends/builtin/pianosample.h"
#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectmanifestparameter.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/types.h"

namespace {

// Semitone offsets relative to A4 (440 Hz), one entry per chromatic key.
// Index 0=C, 1=C#/Db, 2=D, 3=D#/Eb, 4=E, 5=F,
//       6=F#/Gb, 7=G, 8=G#/Ab, 9=A, 10=A#/Bb, 11=B
constexpr std::array<int, 12> kKeySemitoneOffset = {
        -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2};

/// Mixes a pitched mono source into a stereo-interleaved output buffer.
/// pitchRatio > 1 plays faster (higher pitch); < 1 plays slower (lower pitch).
/// Linear interpolation avoids clicks at non-integer source positions.
/// Returns the number of source frames consumed.
std::size_t playPitchedMonoSamplesWithGain(
        std::span<const CSAMPLE> monoSource,
        std::span<CSAMPLE> output,
        CSAMPLE_GAIN gain,
        double pitchRatio) {
    const std::size_t outputFrames =
            output.size() / mixxx::kEngineChannelOutputCount;

    std::size_t sourceFramesConsumed = 0;
    for (std::size_t i = 0; i < outputFrames; ++i) {
        const double sourcePos = static_cast<double>(i) * pitchRatio;
        const auto sourceIndex = static_cast<std::size_t>(sourcePos);

        if (sourceIndex >= monoSource.size()) {
            break;
        }

        CSAMPLE sample;
        if (sourceIndex + 1 < monoSource.size()) {
            const double frac = sourcePos - static_cast<double>(sourceIndex);
            sample = static_cast<CSAMPLE>(
                    monoSource[sourceIndex] * (1.0 - frac) +
                    monoSource[sourceIndex + 1] * frac);
        } else {
            sample = monoSource[sourceIndex];
        }

        const std::size_t outIdx = i * mixxx::kEngineChannelOutputCount;
        output[outIdx] += sample * gain;
        output[outIdx + 1] += sample * gain;

        sourceFramesConsumed = sourceIndex + 1;
    }
    return sourceFramesConsumed;
}

template<class T>
std::span<T> subspanClamped(
        std::span<T> in, typename std::span<T>::size_type offset) {
    return in.subspan(std::min(offset, in.size()));
}

constexpr std::size_t framesPerBeat(
        mixxx::audio::SampleRate sampleRate, double bpm) {
    return static_cast<std::size_t>(
            static_cast<double>(sampleRate) * 60.0 / bpm);
}

/// Returns the sub-span of output starting at the next downbeat,
/// or empty if no beat falls within this buffer.
std::span<CSAMPLE> syncedNoteOutput(
        double beatFractionBufferEnd,
        std::optional<GroupFeatureBeatLength> beatLengthAndScratch,
        const mixxx::EngineParameters& engineParameters,
        std::span<CSAMPLE> output) {
    if (!beatLengthAndScratch.has_value() ||
            beatLengthAndScratch->scratch_rate == 0.0) {
        return {};
    }

    const double beatLength = beatLengthAndScratch->seconds *
            engineParameters.sampleRate() /
            beatLengthAndScratch->scratch_rate;

    const bool needsPreviousBeat = beatLength < 0;
    const double beatToBufferEndFrames = std::abs(beatLength) *
            (needsPreviousBeat ? (1.0 - beatFractionBufferEnd)
                               : beatFractionBufferEnd);
    const std::size_t beatToBufferEndSamples =
            static_cast<std::size_t>(beatToBufferEndFrames) *
            mixxx::kEngineChannelOutputCount;

    if (beatToBufferEndSamples <= output.size()) {
        return output.last(beatToBufferEndSamples);
    }
    return {};
}

std::span<CSAMPLE> unsyncedNoteOutput(
        mixxx::audio::SampleRate sampleRate,
        std::size_t framesSinceLastNote,
        double bpm,
        std::span<CSAMPLE> output) {
    const std::size_t offset =
            framesSinceLastNote % framesPerBeat(sampleRate, bpm);
    return subspanClamped(
            output, offset * mixxx::kEngineChannelOutputCount);
}

} // namespace

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
            "Plays a piano note on every downbeat so you can match it "
            "to the track and identify or verify the musical key."));
    pManifest->setEffectRampsFromDry(true);
    // Metaknob maps to gain; neutral position = 0 dB = 24/27 on the scale.
    pManifest->setMetaknobDefault(24.0 / 27.0);

    EffectManifestParameterPointer key = pManifest->addParameter();
    key->setId(QStringLiteral("key"));
    key->setName(QObject::tr("Key"));
    key->setShortName(QObject::tr("Key"));
    key->setDescription(QObject::tr(
            "Musical key of the piano note (12 chromatic steps).\n"
            "0=C  1=C\u266f/D\u266d  2=D  3=D\u266f/E\u266d  4=E  5=F\n"
            "6=F\u266f/G\u266d  7=G  8=G\u266f/A\u266d  9=A  10=A\u266f/B\u266d  11=B"));
    key->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    key->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    key->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    key->setRange(0.0, 9.0, 11.0);

    EffectManifestParameterPointer tuning = pManifest->addParameter();
    tuning->setId(QStringLiteral("tuning"));
    tuning->setName(QObject::tr("Tuning"));
    tuning->setShortName(QObject::tr("Tune"));
    tuning->setDescription(QObject::tr(
            "A4 standard tuning is 440 Hz.\n"
            "Raise or lower this if the track is tuned\n"
            "above or below 440 Hz\n"
            "(e.g. 432 Hz, 442 Hz, 415 Hz Baroque)."));
    tuning->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    tuning->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    tuning->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    // (440 - 415) / (466 - 415) = 25/51 places 440 Hz at the neutral point.
    tuning->setNeutralPointOnScale(25.0 / 51.0);
    tuning->setRange(415.0, 440.0, 466.0);

    EffectManifestParameterPointer bpm = pManifest->addParameter();
    bpm->setId(QStringLiteral("bpm"));
    bpm->setName(QObject::tr("BPM"));
    bpm->setShortName(QObject::tr("BPM"));
    bpm->setDescription(QObject::tr(
            "Note repeat rate when Sync is disabled."));
    bpm->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bpm->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bpm->setDefaultLinkType(EffectManifestParameter::LinkType::None);
    bpm->setRange(60.0, 120.0, 200.0);

    EffectManifestParameterPointer sync = pManifest->addParameter();
    sync->setId(QStringLiteral("sync"));
    sync->setName(QObject::tr("Sync"));
    sync->setShortName(QObject::tr("Sync"));
    sync->setDescription(QObject::tr(
            "When enabled, the note plays on every detected downbeat.\n"
            "When disabled, the BPM knob controls the note rate."));
    sync->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    sync->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    sync->setRange(0.0, 1.0, 1.0);

    EffectManifestParameterPointer gain = pManifest->addParameter();
    gain->setId(QStringLiteral("gain"));
    gain->setName(QObject::tr("Gain"));
    gain->setShortName(QObject::tr("Gain"));
    gain->setDescription(QObject::tr("Volume of the piano note in dB."));
    gain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    gain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    gain->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    gain->setNeutralPointOnScale(24.0 / 27.0);
    gain->setRange(-24.0, 0.0, 3.0);

    return pManifest;
}

void KeyComparisonEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pKeyParameter = parameters.value(QStringLiteral("key"));
    m_pTuningParameter = parameters.value(QStringLiteral("tuning"));
    m_pBpmParameter = parameters.value(QStringLiteral("bpm"));
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

    if (pOutput != pInput) {
        SampleUtil::copy(
                pOutput, pInput, engineParameters.samplesPerBuffer());
    }

    const std::span<CSAMPLE> output(
            pOutput, engineParameters.samplesPerBuffer());
    const std::span<const CSAMPLE> pianoSample =
            pianoSampleForSampleRate(engineParameters.sampleRate());

    const bool shouldSync = m_pSyncParameter->toBool();
    const bool hasBeatInfo =
            groupFeatures.beat_length.has_value() &&
            groupFeatures.beat_fraction_buffer_end.has_value();

    // Skip the first note on enable when synced so we don't fire mid-beat.
    if (enableState == EffectEnableState::Enabling) {
        pGroupState->framesSinceLastNote =
                (shouldSync && hasBeatInfo) ? pianoSample.size() : 0;
    }

    // pitchRatio = 2^(keySemitones/12) * (tuningHz/440)
    // The key shift moves in equal-tempered steps; tuning scales linearly.
    const int keyIndex = std::clamp(
            static_cast<int>(std::round(m_pKeyParameter->value())),
            0,
            static_cast<int>(kKeySemitoneOffset.size()) - 1);
    const double pitchRatio =
            std::pow(2.0, kKeySemitoneOffset[keyIndex] / 12.0) *
            (m_pTuningParameter->value() / 440.0);

    const CSAMPLE_GAIN gain =
            db2ratio(static_cast<float>(m_pGainParameter->value()));
    const double bpm = m_pBpmParameter->value();

    // Continue playing any remainder of the note started in the previous buffer.
    // Source offset accounts for pitch so we read at the correct position.
    const auto sourceOffset = static_cast<std::size_t>(
            static_cast<double>(pGroupState->framesSinceLastNote) *
            pitchRatio);
    playPitchedMonoSamplesWithGain(
            subspanClamped(pianoSample, sourceOffset),
            output,
            gain,
            pitchRatio);
    pGroupState->framesSinceLastNote += engineParameters.framesPerBuffer();

    const std::span<CSAMPLE> noteOffset = shouldSync && hasBeatInfo
            ? syncedNoteOutput(
                      *groupFeatures.beat_fraction_buffer_end,
                      groupFeatures.beat_length,
                      engineParameters,
                      output)
            : unsyncedNoteOutput(
                      engineParameters.sampleRate(),
                      pGroupState->framesSinceLastNote,
                      bpm,
                      output);

    if (!noteOffset.empty()) {
        pGroupState->framesSinceLastNote = playPitchedMonoSamplesWithGain(
                pianoSample, noteOffset, gain, pitchRatio);
    }
}
