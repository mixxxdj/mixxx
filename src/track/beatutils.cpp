#include "track/beatutils.h"

#include <QtDebug>

#include "util/math.h"

namespace {

// When ironing the grid for long sequences of const tempo we use
// a 25 ms tolerance because this small of a difference is inaudible
// This is > 2 * 12 ms, the step width of the QM beat detector
constexpr double kMaxSecsPhaseError = 0.025;
// This is set to avoid to use a constant region during an offset shift.
// That happens for instance when the beat instrument changes.
constexpr double kMaxSecsPhaseErrorSum = 0.1;
constexpr int kMaxOutliersCount = 1;
constexpr int kMinRegionBeatCount = 16;

} // namespace

mixxx::Bpm BeatUtils::calculateAverageBpm(int numberOfBeats,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::FramePos lowerFrame,
        mixxx::audio::FramePos upperFrame) {
    mixxx::audio::FrameDiff_t frames = upperFrame - lowerFrame;
    DEBUG_ASSERT(frames > 0);
    if (numberOfBeats < 1) {
        return {};
    }
    return mixxx::Bpm(60.0 * numberOfBeats * sampleRate / frames);
}

mixxx::Bpm BeatUtils::calculateBpm(
        const QVector<mixxx::audio::FramePos>& beats,
        mixxx::audio::SampleRate sampleRate) {
    if (beats.size() < 2) {
        return {};
    }

    // If we don't have enough beats for our regular approach, just divide the #
    // of beats by the duration in minutes.
    if (beats.size() < kMinRegionBeatCount) {
        return calculateAverageBpm(beats.size() - 1, sampleRate, beats.first(), beats.last());
    }

    QVector<BeatUtils::ConstRegion> constantRegions =
            retrieveConstRegions(beats, sampleRate);
    return makeConstBpm(constantRegions, sampleRate, nullptr);
}

QVector<BeatUtils::ConstRegion> BeatUtils::retrieveConstRegions(
        const QVector<mixxx::audio::FramePos>& coarseBeats,
        mixxx::audio::SampleRate sampleRate) {
    DEBUG_ASSERT(sampleRate.isValid());
    if (coarseBeats.size() < 2) {
        // Cannot infer a tempo for less than 2 beats
        return {};
    }

    // The QM Beat detector has a step size of 512 frames @ 44100 Hz. This means that
    // Single beats have has a jitter of +- 12 ms around the actual position.
    // Expressed in BPM it means we have for instance steps of these BPM value around 120 BPM
    // 117.454 - 120.185 - 123.046 - 126.048
    // A pure electronic 120.000 BPM track will have many 120,185 BPM beats and a few
    // 117,454 BPM beats to adjust the collected offset.
    // This function irons these adjustment beats by adjusting every beat to the average of
    // a likely constant region.

    // Therefore we loop through the coarse beats and calculate the average beat
    // length from the first beat.
    // A inner loop checks for outliers using the momentary average as beat length.
    // once we have found an average with only single outliers, we store the beats using the
    // current average to adjust them by up to +-12 ms.
    // Than we start with the region from the found beat to the end.

    const mixxx::audio::FrameDiff_t maxPhaseError = kMaxSecsPhaseError * sampleRate;
    const mixxx::audio::FrameDiff_t maxPhaseErrorSum = kMaxSecsPhaseErrorSum * sampleRate;
    int leftIndex = 0;
    int rightIndex = coarseBeats.size() - 1;

    QVector<ConstRegion> constantRegions;
    while (leftIndex < coarseBeats.size() - 1) {
        DEBUG_ASSERT(rightIndex > leftIndex);
        mixxx::audio::FrameDiff_t meanBeatLength =
                (coarseBeats[rightIndex] - coarseBeats[leftIndex]) /
                (rightIndex - leftIndex);
        int outliersCount = 0;
        mixxx::audio::FramePos ironedBeat = coarseBeats[leftIndex];
        mixxx::audio::FrameDiff_t phaseErrorSum = 0;
        int i = leftIndex + 1;
        for (; i <= rightIndex; ++i) {
            ironedBeat += meanBeatLength;
            const mixxx::audio::FrameDiff_t phaseError = ironedBeat - coarseBeats[i];
            phaseErrorSum += phaseError;
            if (fabs(phaseError) > maxPhaseError) {
                outliersCount++;
                if (outliersCount > kMaxOutliersCount ||
                        i == leftIndex + 1) { // the first beat must not be an outlier.
                    // region is not const.
                    break;
                }
            }
            if (fabs(phaseErrorSum) > maxPhaseErrorSum) {
                // we drift away in one direction, the meanBeatLength is not optimal.
                break;
            }
        }
        if (i > rightIndex) {
            // Verify that the first an the last beat are not correction beats in the same direction
            // This would bend meanBeatLength unfavorably away from the optimum.
            mixxx::audio::FrameDiff_t regionBorderError = 0;
            if (rightIndex > leftIndex + 2) {
                const mixxx::audio::FrameDiff_t firstBeatLength =
                        coarseBeats[leftIndex + 1] - coarseBeats[leftIndex];
                const mixxx::audio::FrameDiff_t lastBeatLength =
                        coarseBeats[rightIndex] - coarseBeats[rightIndex - 1];
                regionBorderError = fabs(firstBeatLength + lastBeatLength - (2 * meanBeatLength));
            }
            if (regionBorderError < maxPhaseError / 2) {
                // We have found a constant enough region.
                const mixxx::audio::FramePos firstBeat = coarseBeats[leftIndex];
                // store the regions for the later stages
                constantRegions.append({firstBeat, meanBeatLength});
                // continue with the next region.
                leftIndex = rightIndex;
                rightIndex = coarseBeats.size() - 1;
                continue;
            }
        }
        // Try a by one beat smaller region
        rightIndex--;
    }

    // Add a final region with zero length to mark the end.
    constantRegions.append({coarseBeats.last(), 0});
    return constantRegions;
}

// static
mixxx::Bpm BeatUtils::makeConstBpm(
        const QVector<BeatUtils::ConstRegion>& constantRegions,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::FramePos* pFirstBeat) {
    DEBUG_ASSERT(!constantRegions.isEmpty());
    DEBUG_ASSERT(sampleRate.isValid());
    DEBUG_ASSERT(!pFirstBeat || pFirstBeat->isValid());
    // We assume here the track was recorded with an unhear-able static metronome.
    // This metronome is likely at a full BPM.
    // The track may has intros, outros and bridges without detectable beats.
    // In these regions the detected beat might is floating around and is just wrong.
    // The track may also has regions with different rhythm giving instruments. They
    // have a different shape of onsets and introduce a static beat offset.
    // The track may also have break beats or other issues that makes the detector
    // hook onto a beat that is by an integer fraction off the original metronome.

    // This code aims to find the static metronome and a phase offset.

    // Find the longest region somewhere in the middle of the track to start with.
    // At least this region will be have finally correct annotated beats.

    // Note: This function is channel count independent. All sample based values in
    // this functions are based on frames.

    int midRegionIndex = 0;
    mixxx::audio::FrameDiff_t longestRegionLength = 0;
    mixxx::audio::FrameDiff_t longestRegionBeatLength = 0;
    for (int i = 0; i < constantRegions.size() - 1; ++i) {
        mixxx::audio::FrameDiff_t length =
                constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        if (length > longestRegionLength) {
            longestRegionLength = length;
            longestRegionBeatLength = constantRegions[i].beatLength;
            midRegionIndex = i;
        }
        //qDebug() << i << length << constantRegions[i].beatLength;
    }

    if (longestRegionLength == 0) {
        // Could not infer a tempo
        return {};
    }

    int longestRegionNumberOfBeats = static_cast<int>(
            (longestRegionLength / longestRegionBeatLength) + 0.5);
    mixxx::audio::FrameDiff_t longestRegionBeatLengthMin = longestRegionBeatLength -
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
    mixxx::audio::FrameDiff_t longestRegionBeatLengthMax = longestRegionBeatLength +
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);

    int startRegionIndex = midRegionIndex;

    // Find a region at the beginning of the track with a similar tempo and phase
    for (int i = 0; i < midRegionIndex; ++i) {
        const mixxx::audio::FrameDiff_t length =
                constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        const int numberOfBeats = static_cast<int>((length / constantRegions[i].beatLength) + 0.5);
        if (numberOfBeats < kMinRegionBeatCount) {
            // Request short regions, too unstable.
            continue;
        }
        const mixxx::audio::FrameDiff_t thisRegionBeatLengthMin = constantRegions[i].beatLength -
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        const mixxx::audio::FrameDiff_t thisRegionBeatLengthMax = constantRegions[i].beatLength +
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        // check if the tempo of the longest region is part of the rounding range of this region
        if (longestRegionBeatLength > thisRegionBeatLengthMin &&
                longestRegionBeatLength < thisRegionBeatLengthMax) {
            // Now check if both regions are at the same phase.
            const mixxx::audio::FrameDiff_t newLongestRegionLength =
                    constantRegions[midRegionIndex + 1].firstBeat -
                    constantRegions[i].firstBeat;

            mixxx::audio::FrameDiff_t beatLengthMin = math_max(
                    longestRegionBeatLengthMin, thisRegionBeatLengthMin);
            mixxx::audio::FrameDiff_t beatLengthMax = math_min(
                    longestRegionBeatLengthMax, thisRegionBeatLengthMax);

            const int maxNumberOfBeats =
                    static_cast<int>(round(newLongestRegionLength / beatLengthMin));
            const int minNumberOfBeats =
                    static_cast<int>(round(newLongestRegionLength / beatLengthMax));

            if (minNumberOfBeats != maxNumberOfBeats) {
                // Ambiguous number of beats, find a closer region.
                continue;
            }
            const int numberOfBeats = minNumberOfBeats;
            const mixxx::audio::FrameDiff_t newBeatLength = newLongestRegionLength / numberOfBeats;
            if (newBeatLength > longestRegionBeatLengthMin &&
                    newBeatLength < longestRegionBeatLengthMax) {
                longestRegionLength = newLongestRegionLength;
                longestRegionBeatLength = newBeatLength;
                longestRegionNumberOfBeats = numberOfBeats;
                longestRegionBeatLengthMin = longestRegionBeatLength -
                        ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
                longestRegionBeatLengthMax = longestRegionBeatLength +
                        ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
                startRegionIndex = i;
                break;
            }
        }
    }

    // Find a region at the end of the track with similar tempo and phase
    for (int i = constantRegions.size() - 2; i > midRegionIndex; --i) {
        const mixxx::audio::FrameDiff_t length =
                constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        const int numberOfBeats = static_cast<int>((length / constantRegions[i].beatLength) + 0.5);
        if (numberOfBeats < kMinRegionBeatCount) {
            continue;
        }
        const mixxx::audio::FrameDiff_t thisRegionBeatLengthMin = constantRegions[i].beatLength -
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        const mixxx::audio::FrameDiff_t thisRegionBeatLengthMax = constantRegions[i].beatLength +
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        if (longestRegionBeatLength > thisRegionBeatLengthMin &&
                longestRegionBeatLength < thisRegionBeatLengthMax) {
            // Now check if both regions are at the same phase.
            const mixxx::audio::FrameDiff_t newLongestRegionLength =
                    constantRegions[i + 1].firstBeat -
                    constantRegions[startRegionIndex].firstBeat;

            mixxx::audio::FrameDiff_t minBeatLength = math_max(
                    longestRegionBeatLengthMin, thisRegionBeatLengthMin);
            mixxx::audio::FrameDiff_t maxBeatLength = math_min(
                    longestRegionBeatLengthMax, thisRegionBeatLengthMax);

            const int maxNumberOfBeats =
                    static_cast<int>(round(newLongestRegionLength / minBeatLength));
            const int minNumberOfBeats =
                    static_cast<int>(round(newLongestRegionLength / maxBeatLength));

            if (minNumberOfBeats != maxNumberOfBeats) {
                // Ambiguous number of beats, find a closer region.
                continue;
            }
            const int numberOfBeats = minNumberOfBeats;
            const mixxx::audio::FrameDiff_t newBeatLength = newLongestRegionLength / numberOfBeats;
            if (newBeatLength > longestRegionBeatLengthMin &&
                    newBeatLength < longestRegionBeatLengthMax) {
                longestRegionLength = newLongestRegionLength;
                longestRegionBeatLength = newBeatLength;
                longestRegionNumberOfBeats = numberOfBeats;
                break;
            }
        }
    }

    longestRegionBeatLengthMin = longestRegionBeatLength -
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
    longestRegionBeatLengthMax = longestRegionBeatLength +
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);

    // qDebug() << startRegion << midRegion << constantRegions.size()
    //         << longestRegionLength << "<<<<<<<<<<<<<<<<<<<<<<<<<";

    //qDebug() << "First beat" << constantRegions[startRegion].firstBeat;
    //qDebug() << longestRegionLength << longestRegionNumberOfBeats;

    // Create a const region region form the first beat of the first region to the last beat of the last region.

    const mixxx::Bpm minRoundBpm = mixxx::Bpm(60.0 * sampleRate / longestRegionBeatLengthMax);
    const mixxx::Bpm maxRoundBpm = mixxx::Bpm(60.0 * sampleRate / longestRegionBeatLengthMin);
    const mixxx::Bpm centerBpm = mixxx::Bpm(60.0 * sampleRate / longestRegionBeatLength);

    //qDebug() << "minRoundBpm" << minRoundBpm;
    //qDebug() << "maxRoundBpm" << maxRoundBpm;
    const mixxx::Bpm roundBpm = roundBpmWithinRange(minRoundBpm, centerBpm, maxRoundBpm);

    if (pFirstBeat) {
        // Move the first beat as close to the start of the track as we can. This is
        // a constant beatgrid so "first beat" only affects the anchor point where
        // bpm adjustments are made.
        // This is a temporary fix, ideally the anchor point for the BPM grid should
        // be the first proper downbeat, or perhaps the CUE point.
        const double roundedBeatLength = 60.0 * sampleRate / roundBpm.value();
        *pFirstBeat = mixxx::audio::FramePos(
                fmod(constantRegions[startRegionIndex].firstBeat.value(),
                        roundedBeatLength));
    }
    return roundBpm;
}

// static
std::optional<mixxx::Bpm> BeatUtils::trySnap(mixxx::Bpm minBpm,
        mixxx::Bpm centerBpm,
        mixxx::Bpm maxBpm,
        double fraction) {
    mixxx::Bpm snapBpm = mixxx::Bpm(round(centerBpm.value() * fraction) / fraction);
    if (snapBpm > minBpm && snapBpm < maxBpm) {
        return snapBpm;
    }
    return std::nullopt;
};

// static
mixxx::Bpm BeatUtils::roundBpmWithinRange(
        mixxx::Bpm minBpm, mixxx::Bpm centerBpm, mixxx::Bpm maxBpm) {
    // First try to snap to a full integer BPM
    // FIXME: calling bpm.value() without checking bpm.isValid()
    std::optional<mixxx::Bpm> snapBpm = trySnap(minBpm, centerBpm, maxBpm, 1.0);
    if (snapBpm) {
        return *snapBpm;
    }

    // 0.5 BPM are only reasonable if the double value is not insane
    // else other factors below are more typical
    if (centerBpm < mixxx::Bpm(85.0)) {
        // this can be actually up to 175 BPM
        // allow halve BPM values
        snapBpm = trySnap(minBpm, centerBpm, maxBpm, 2.0);
        if (snapBpm) {
            return *snapBpm;
        }
    }

    if (centerBpm > mixxx::Bpm(127.0)) {
        // optimize for 2/3 going down to 85
        // else other factors below are more typical
        snapBpm = trySnap(minBpm, centerBpm, maxBpm, 2.0 / 3.0);
        if (snapBpm) {
            return *snapBpm;
        }
    }

    // try to snap to a 1/3 Bpm
    // This covers all sorts of 3/2 and 3/4 multipliers
    snapBpm = trySnap(minBpm, centerBpm, maxBpm, 3.0);
    if (snapBpm) {
        return *snapBpm;
    }

    // try to snap to a 1/12 Bpm
    // This covers all other sorts of typical multipliers
    snapBpm = trySnap(minBpm, centerBpm, maxBpm, 12.0);
    if (snapBpm) {
        return *snapBpm;
    }

    // else give up and use the original BPM value.
    return centerBpm;
}

// static
QVector<mixxx::audio::FramePos> BeatUtils::getBeats(
        const QVector<BeatUtils::ConstRegion>& constantRegions) {
    QVector<mixxx::audio::FramePos> beats;
    for (int i = 0; i < constantRegions.size() - 1; ++i) {
        mixxx::audio::FramePos beat = constantRegions[i].firstBeat;
        constexpr double epsilon = 100; // Protection against tiny beats due rounding
        while (beat < constantRegions[i + 1].firstBeat - epsilon) {
            beats.append(beat);
            beat += constantRegions[i].beatLength;
        }
    }
    if (constantRegions.size() > 0) {
        beats.append(constantRegions.last().firstBeat);
    }
    return beats;
}

// static
mixxx::audio::FramePos BeatUtils::adjustPhase(
        mixxx::audio::FramePos firstBeat,
        mixxx::Bpm bpm,
        mixxx::audio::SampleRate sampleRate,
        const QVector<mixxx::audio::FramePos>& beats) {
    // FIXME: calling bpm.value() without checking bpm.isValid()
    const double beatLength = 60 * sampleRate / bpm.value();
    const mixxx::audio::FramePos startOffset =
            mixxx::audio::FramePos(fmod(firstBeat.value(), beatLength));
    mixxx::audio::FrameDiff_t offsetAdjust = 0;
    double offsetAdjustCount = 0;
    for (const auto& beat : beats) {
        mixxx::audio::FrameDiff_t offset = fmod(beat - startOffset, beatLength);
        if (offset > beatLength / 2) {
            offset -= beatLength;
        }
        if (abs(offset) < (kMaxSecsPhaseError * sampleRate)) {
            offsetAdjust += offset;
            offsetAdjustCount++;
        }
    }
    offsetAdjust /= offsetAdjustCount;
    qDebug() << "adjusting phase by" << offsetAdjust;
    DEBUG_ASSERT(abs(offsetAdjust) < (kMaxSecsPhaseError * sampleRate));

    return firstBeat + offsetAdjust;
}
