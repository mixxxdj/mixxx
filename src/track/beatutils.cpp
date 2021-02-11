/*
 * beatutils.cpp
 *
 *  Created on: 30/nov/2011
 *      Author: vittorio
 */

#include <algorithm>
#include <QtDebug>
#include <QString>
#include <QList>
#include <QMap>

#include "track/beatutils.h"
#include "util/math.h"

namespace {

// we are generous and assume the global_BPM to be at most 0.05 BPM far away
// from the correct one
#define BPM_ERROR 0.05

// the raw beatgrid is divided into blocks of size N from which the local bpm is
// computed. Tweaked from 8 to 12 which improves the BPM accuracy for 'problem songs'.
#define N 12

static bool sDebug = false;

const int kHistogramDecimalPlaces = 2;
const double kHistogramDecimalScale = pow(10.0, kHistogramDecimalPlaces);
const double kBpmFilterTolerance = 1.0;

// When ironing the grid for long sequences of const tempo we use
// a 25 ms tolerance because this small of a difference is inaudible
// This is > 2 * 12 ms, the step width of the QM beat detector
constexpr double kMaxSecsPhaseError = 0.025;
// This is set to avoid to use a constant region during an offset shift.
// That happens for instance when the beat instrument changes.
constexpr double kMaxSecsPhaseErrorSum = 0.1;
constexpr int kMaxOutlierCount = 1;
constexpr int kMinRegionBeatCount = 16;

} // namespace


// Given a sorted set of numbers, find the sample median.
// http://en.wikipedia.org/wiki/Median#The_sample_median
double BeatUtils::computeSampleMedian(const QList<double>& sortedItems) {
    if (sortedItems.empty()) {
        return 0.0;
    }

    // When there are an even number of elements, the sample median is the mean
    // of the middle 2 elements.
    if (sortedItems.size() % 2 == 0) {
        int item_position = sortedItems.size() / 2;
        double item_value1 = sortedItems.at(item_position - 1);
        double item_value2 = sortedItems.at(item_position);
        return (item_value1 + item_value2) / 2.0;
    }

    // When there are an odd number of elements, find the {(n+1)/2}th item in
    // the sorted list.
    int item_position = (sortedItems.size() + 1) / 2;
    return sortedItems.at(item_position - 1);
}

QList<double> BeatUtils::computeWindowedBpmsAndFrequencyHistogram(
        const QVector<double>& beats,
        const int windowSize,
        const int windowStep,
        const int sampleRate,
        QMap<double, int>* frequencyHistogram) {
    QList<double> averageBpmList;
    for (int i = windowSize; i < beats.size(); i += windowStep) {
        //get start and end sample of the beats
        double start_sample = beats.at(i - windowSize);
        double end_sample = beats.at(i);

        // Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample) / sampleRate;
        if (time == 0) {
            continue;
        }
        double localBpm = 60.0 * windowSize / time;

        // round BPM to have two decimal places
        double roundedBpm = floor(localBpm * kHistogramDecimalScale + 0.5) /
                kHistogramDecimalScale;

        // add to local BPM to list and increment frequency count
        averageBpmList << roundedBpm;
        (*frequencyHistogram)[roundedBpm] += 1;
    }
    return averageBpmList;
}

double BeatUtils::computeFilteredWeightedAverage(
        const QMap<double, int>& frequencyTable,
        const double filterCenter,
        const double filterTolerance,
        QMap<double, int>* filteredFrequencyTable) {
    double filterWeightedAverage = 0.0;
    int filterSum = 0;
    QMapIterator<double, int> i(frequencyTable);

    while (i.hasNext()) {
        i.next();
        const double value = i.key();
        const int frequency = i.value();

        if (fabs(value - filterCenter) <= filterTolerance) {
            // TODO(raffitea): Why > 1 ?
            if (i.value() > 1) {
                filterSum += frequency;
                filterWeightedAverage += value * frequency;
                filteredFrequencyTable->insert(i.key(), frequency);
                if (sDebug) {
                    qDebug() << "Filtered Table:" << value
                             << "Frequency:" << frequency;
                }
            }
        }
    }
    if (sDebug) {
        qDebug() << "Sum of filtered frequencies: " << filterSum;
    }
    if (filterSum == 0) {
        return filterCenter;
    }
    return filterWeightedAverage / static_cast<double>(filterSum);
}

double BeatUtils::calculateBpm(const QVector<double>& beats,
        const mixxx::audio::SampleRate& sampleRate,
        int min_bpm,
        int max_bpm) {
    int SampleRate = sampleRate;
    /*
     * Let's compute the average local
     * BPM for N subsequent beats.
     * The average BPMs are
     * added to a list from which the statistical
     * median is computed
     *
     * N=12 seems to work great; We coincide with Traktor's
     * BPM value in many case but not worse than +-0.2 BPM
     */
    /*
     * Just to demonstrate how you would count the beats manually
     *
     *    Beat numbers:   1  2  3  4   5  6  7  8    9
     *    Beat positions: ?  ?  ?  ?  |?  ?  ?  ?  | ?
     *
     * Usually one measures the time of N beats. One stops the timer just before
     * the (N+1)th beat begins.  The BPM is then computed by 60*N/<time needed
     * to count N beats (in seconds)>
     *
     * Although beat tracking through QM is promising, the local average BPM of
     * 4 beats varies frequently by +-2 BPM.  Sometimes there N subsequent beats
     * in the grid that are computed wrongly by QM.
     *
     * Their local BPMs can be considered as outliers which would influence the
     * BPM computation negatively. To exclude outliers, we select the median BPM
     * over a window of N subsequent beats.

     * To do this, we take the average local BPM for every N subsequent
     * beats. We then sort the averages and take the middle to find the median
     * BPM.
     */

    if (beats.size() < 2) {
        return 0;
    }

    // If we don't have enough beats for our regular approach, just divide the #
    // of beats by the duration in minutes.
    if (beats.size() <= N) {
        return 60.0 * (beats.size()-1) * SampleRate / (beats.last() - beats.first());
    }

    QMap<double, int> frequency_table;
    QList<double> average_bpm_list = computeWindowedBpmsAndFrequencyHistogram(
        beats, N, 1, SampleRate, &frequency_table);

    // Get the median BPM.
    std::sort(average_bpm_list.begin(), average_bpm_list.end());
    const double median = computeSampleMedian(average_bpm_list);

    /*
     * Okay, let's consider the median an estimation of the BPM To not solely
     * rely on the median, we build the average weighted value of all bpm values
     * being at most +-1 BPM from the median away.  Please note, this has
     * improved the BPM: While relying on median only we may have a deviation of
     * about +-0.2 BPM, taking into account BPM values around the median leads
     * to deviation of +- 0.05 Please also note that this value refers to
     * electronic music, but to be honest, the BPM detection of Traktor and Co
     * work best with electronic music, too. But BPM detection for
     * non-electronic music isn't too bad.
     */

    //qDebug() << "BPM range between " << min_bpm << " and " << max_bpm;

    // a subset of the 'frequency_table', where the bpm values are +-1 away from
    // the median average BPM.
    QMap<double, int> filtered_bpm_frequency_table;
    const double filterWeightedAverageBpm = computeFilteredWeightedAverage(
        frequency_table, median, kBpmFilterTolerance, &filtered_bpm_frequency_table);

    if (sDebug) {
        qDebug() << "Statistical median BPM: " << median;
        qDebug() << "Weighted Avg of BPM values +- 1BPM from the media"
                 << filterWeightedAverageBpm;
    }

    /*
     * Although we have a minimal deviation of about +- 0.05 BPM units compared
     * to Traktor, this deviation may cause the beat grid to look unaligned,
     * especially at the end of a track.  Let's try to get the BPM 'perfect' :-)
     *
     * Idea: Iterate over the original beat set where some detected beats may be
     * wrong. The beat is considered 'correct' if the beat position is within
     * epsilon of a beat grid obtained by the global BPM.
     *
     * If the beat turns out correct, we can compute the error in BPM units.
     * E.g., we can check the original beat position after 60 seconds. Ideally,
     * the approached beat is just a couple of samples away, i.e., not worse
     * than 0.05 BPM units.  The distance between these two samples can be used
     * for BPM error correction.
     */

     double perfect_bpm = 0;
     double firstCorrectBeatSample = beats.first();
     bool foundFirstCorrectBeat = false;

     int counter = 0;
     int perfectBeats = 0;
     for (int i = N; i < beats.size(); i += 1) {
         // get start and end sample of the beats
         double beat_start = beats.at(i-N);
         double beat_end = beats.at(i);

         // Time needed to count a bar (N beats)
         double time = (beat_end - beat_start) / SampleRate;
         if (time == 0) {
             continue;
         }
         double local_bpm = 60.0 * N / time;
         // round BPM to have two decimal places
         local_bpm = floor(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;

         //qDebug() << "Local BPM beat " << i << ": " << local_bpm;
         if (!foundFirstCorrectBeat &&
             filtered_bpm_frequency_table.contains(local_bpm) &&
             fabs(local_bpm - filterWeightedAverageBpm) < BPM_ERROR) {
             firstCorrectBeatSample = beat_start;
             foundFirstCorrectBeat = true;
             if (sDebug) {
                 qDebug() << "Beat #" << (i - N)
                          << "is considered as reference beat with BPM:"
                          << local_bpm;
             }
         }
         if (foundFirstCorrectBeat) {
             if (counter == 0) {
                 counter = N;
             } else {
                 counter += 1;
             }
             double time2 = (beat_end - firstCorrectBeatSample) / SampleRate;
             double correctedBpm = 60 * counter / time2;

             if (fabs(correctedBpm - filterWeightedAverageBpm) <= BPM_ERROR) {
                 perfect_bpm += correctedBpm;
                 ++perfectBeats;
                 if (sDebug) {
                     qDebug() << "Beat #" << (i-N)
                              << "is considered as correct -->BPM improved to:"
                              << correctedBpm;
                 }
             }
         }
     }

     const double perfectAverageBpm = perfectBeats > 0 ?
             perfect_bpm / perfectBeats : filterWeightedAverageBpm;

     // Round values that are within BPM_ERROR of a whole number.
     const double rounded_bpm = floor(perfectAverageBpm + 0.5);
     const double bpm_diff = fabs(rounded_bpm - perfectAverageBpm);
     bool perform_rounding = (bpm_diff <= BPM_ERROR);

     // Finally, restrict the BPM to be within min_bpm and max_bpm.
     const double maybeRoundedBpm = perform_rounding ? rounded_bpm : perfectAverageBpm;
     const double constrainedBpm = constrainBpm(maybeRoundedBpm, min_bpm, max_bpm, false);

     if (sDebug) {
         qDebug() << "SampleMedianBpm=" << median;
         qDebug() << "FilterWeightedAverageBpm=" << filterWeightedAverageBpm;
         qDebug() << "Perfect BPM=" << perfectAverageBpm;
         qDebug() << "Rounded Perfect BPM=" << rounded_bpm;
         qDebug() << "Rounded difference=" << bpm_diff;
         qDebug() << "Perform rounding=" << perform_rounding;
         qDebug() << "Constrained to Range [" << min_bpm << "," << max_bpm << "]=" << constrainedBpm;
     }
     return constrainedBpm;
}

QVector<BeatUtils::ConstRegion> BeatUtils::retrieveConstRegions(
        const QVector<double>& coarseBeats,
        const mixxx::audio::SampleRate& sampleRate) {
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

    QVector<ConstRegion> constantRegions;
    if (!coarseBeats.size()) {
        // no beats
        return constantRegions;
    }

    double maxPhaseError = kMaxSecsPhaseError * sampleRate;
    double maxPhaseErrorSum = kMaxSecsPhaseErrorSum * sampleRate;
    int leftIndex = 0;
    int rightIndex = coarseBeats.size() - 1;

    while (leftIndex < coarseBeats.size() - 1) {
        DEBUG_ASSERT(rightIndex > leftIndex);
        double meanBeatLength =
                (coarseBeats[rightIndex] - coarseBeats[leftIndex]) /
                (rightIndex - leftIndex);
        int outliersCount = 0;
        double ironedBeat = coarseBeats[leftIndex];
        double phaseErrorSum = 0;
        int i = leftIndex + 1;
        for (; i <= rightIndex; ++i) {
            ironedBeat += meanBeatLength;
            double phaseError = ironedBeat - coarseBeats[i];
            phaseErrorSum += phaseError;
            if (fabs(phaseError) > maxPhaseError) {
                outliersCount++;
                if (outliersCount > kMaxOutlierCount ||
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
            // We have found a constant enough region.
            double firstBeat = coarseBeats[leftIndex];
            // store the regions for the later stages
            constantRegions.append({firstBeat, meanBeatLength});
            // continue with the next region.
            leftIndex = rightIndex;
            rightIndex = coarseBeats.size() - 1;
            continue;
        }
        // Try a by one beat smaller region
        rightIndex--;
    }

    // Add a final region with zero length to mark the end.
    constantRegions.append({coarseBeats[coarseBeats.size() - 1], 0});
    return constantRegions;
}

// static
double BeatUtils::makeConstBpm(
        const QVector<BeatUtils::ConstRegion>& constantRegions,
        const mixxx::audio::SampleRate& sampleRate,
        double* pFirstBeat) {
    // We assume her the track was recorded with an unhear-able static metronome.
    // This metronome is likely at a full BPM.
    // The track may has intros, outros and bridges without detectable beats.
    // In these regions the detected beat might is floating around and is just wrong.
    // The track may also has regions with different Rythm giving Instruments. They
    // have a different shape of onsets and introduce a static beat offset.
    // The track may also have break beats or other issues that makes the detector
    // hook onto a beat that is by an integer fraction off the original metronome.

    // This code aims to find the static metronome and a phase offset.

    // Find the longest region somwher in the middle of the track to start with.
    // At least this region will be have finally correct annotated beats.
    int midRegion = 0;
    double longesRegionLength = 0;
    double longesRegionBeatLenth = 0;
    for (int i = 0; i < constantRegions.size() - 1; ++i) {
        double length = constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        if (length > longesRegionLength) {
            longesRegionLength = length;
            longesRegionBeatLenth = constantRegions[i].beatLength;
            midRegion = i;
        }
        qDebug() << i << length << constantRegions[i].beatLength;
    }

    if (longesRegionLength == 0) {
        // no betas, we default to
        return 128;
    }

    int longestRegionNumberOfBeats = static_cast<int>(
            (longesRegionLength / longesRegionBeatLenth) + 0.5);
    double longestRegionMinRoundSamples = longesRegionBeatLenth -
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
    double longestRegionMaxRoundSamples = longesRegionBeatLenth +
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);

    int startRegion = midRegion;

    // Find a region at the beginning of the track with a similar tempo and phase
    for (int i = 0; i < midRegion; ++i) {
        double length = constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        int numberOfBeats = static_cast<int>((length / constantRegions[i].beatLength) + 0.5);
        if (numberOfBeats < kMinRegionBeatCount) {
            // Request short regions, too unstable.
            continue;
        }
        double minRoundSamples = constantRegions[i].beatLength -
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        double maxRoundSamples = constantRegions[i].beatLength +
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        // check if the tempo of the longest region is part of the rounding range of this region
        if (longesRegionBeatLenth > minRoundSamples &&
                longesRegionBeatLenth < maxRoundSamples) {
            // Now check if both regions are at the same phase.
            double newLength = constantRegions[midRegion + 1].firstBeat -
                    constantRegions[i].firstBeat;
            int numberOfOldBeats = static_cast<int>((newLength / longesRegionBeatLenth) + 0.5);
            double newBeatLength = newLength / numberOfOldBeats;
            if (newBeatLength > longestRegionMinRoundSamples &&
                    newBeatLength < longestRegionMaxRoundSamples) {
                longesRegionLength = newLength;
                longesRegionBeatLenth = newBeatLength;
                longestRegionNumberOfBeats = numberOfOldBeats;
                startRegion = i;
                break;
            }
        }
    }

    longestRegionMinRoundSamples = longesRegionBeatLenth -
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
    longestRegionMaxRoundSamples = longesRegionBeatLenth +
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);

    int endRegion = midRegion;

    // Find a region at the end of the track with similar tempo and phase
    for (int i = constantRegions.size() - 2; i > midRegion; --i) {
        double length = constantRegions[i + 1].firstBeat - constantRegions[i].firstBeat;
        int numberOfBeats = static_cast<int>((length / constantRegions[i].beatLength) + 0.5);
        if (numberOfBeats < kMinRegionBeatCount) {
            continue;
        }
        double minRoundSamples = constantRegions[i].beatLength -
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        double maxRoundSamples = constantRegions[i].beatLength +
                ((kMaxSecsPhaseError * sampleRate) / numberOfBeats);
        if (longesRegionLength > minRoundSamples &&
                longesRegionLength < maxRoundSamples) {
            // Now check if both regions are at the same phase.
            double newLength = constantRegions[i + 1].firstBeat -
                    constantRegions[startRegion].firstBeat;
            int numberOfOldBeats = static_cast<int>((newLength / longesRegionBeatLenth) + 0.5);
            double newBeatLength = newLength / numberOfOldBeats;
            if (newBeatLength > longestRegionMinRoundSamples &&
                    newBeatLength < longestRegionMaxRoundSamples) {
                longesRegionLength = newLength;
                longesRegionBeatLenth = newBeatLength;
                longestRegionNumberOfBeats = numberOfOldBeats;
                endRegion = i;
                break;
            }
        }
    }

    longestRegionMinRoundSamples = longesRegionBeatLenth -
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);
    longestRegionMaxRoundSamples = longesRegionBeatLenth +
            ((kMaxSecsPhaseError * sampleRate) / longestRegionNumberOfBeats);

    qDebug() << startRegion << midRegion << endRegion << constantRegions.size()
             << longesRegionLength << "<<<<<<<<<<<<<<<<<<<<<<<<<";

    qDebug() << "First beat" << constantRegions[startRegion].firstBeat;
    qDebug() << "Last beat" << constantRegions[endRegion + 1].firstBeat;
    qDebug() << longesRegionLength << longestRegionNumberOfBeats;

    // Create a const region region form the first beat of the first region to the last beat of the last region.

    double minRoundBpm = 60 * sampleRate / longestRegionMaxRoundSamples;
    double maxRoundBpm = 60 * sampleRate / longestRegionMinRoundSamples;
    double centerBpm = 60 * sampleRate / longesRegionBeatLenth;

    qDebug() << "minRoundBpm" << minRoundBpm;
    qDebug() << "maxRoundBpm" << maxRoundBpm;
    double roundBpm = roundBpmWithinRange(minRoundBpm, centerBpm, maxRoundBpm);

    if (pFirstBeat) {
        *pFirstBeat = constantRegions[startRegion].firstBeat;
    }
    return roundBpm;
}

// static
double BeatUtils::roundBpmWithinRange(double minBpm, double centerBpm, double maxBpm) {
    // First try to snap to a full integer BPM
    double snapBpm = round(centerBpm);
    if (snapBpm > minBpm && snapBpm < maxBpm) {
        // Success
        return snapBpm;
    }

    // Probe the reasonable multipliers for 0.5
    double roundBpmWidth = maxBpm - minBpm;
    if (roundBpmWidth > 1.0 / 2) {
        // 0.5 BPM are only reasonable if the double value is not insane
        // or the 2/3 value is not too small.
        if (centerBpm < 85) {
            // this cane be actually up to 175 BPM
            // allow halve BPM values
            return round(centerBpm * 2) / 2;
        } else if (centerBpm > 127) {
            // optimize for 2/3 going down to 85
            return round(centerBpm / 3 * 2) * 3 / 2;
        }
    }

    if (roundBpmWidth > 1.0 / 12) {
        // this covers all sorts of 1/2 2/3 and 3/4 multiplier
        return round(centerBpm * 12) / 12;
    } else {
        // We are here if we have more that ~75 beats and ~30 s
        // try to snap to a 1/12 Bpm
        double snapBpm = round(centerBpm * 12) / 12;
        if (snapBpm > minBpm && snapBpm < maxBpm) {
            // Success
            return snapBpm;
        }
        // else give up and use the original BPM value.
    }

    return centerBpm;
}

// static
QVector<double> BeatUtils::getBeats(const QVector<BeatUtils::ConstRegion>& constantRegions) {
    QVector<double> beats;
    for (int i = 0; i < constantRegions.size() - 1; ++i) {
        double beat = constantRegions[i].firstBeat;
        constexpr double epsilon = 100; // Protection against tiny beats due rounding
        while (beat < constantRegions[i + 1].firstBeat - epsilon) {
            beats.append(beat);
            beat += constantRegions[i].beatLength;
        }
    }
    return beats;
}

// static
double BeatUtils::adjustPhase(
        double firstBeat,
        double bpm,
        const mixxx::audio::SampleRate& sampleRate,
        const QVector<double>& beats) {
    double beatLength = 60 * sampleRate / bpm;
    double startOffset = fmod(firstBeat, beatLength);
    double offsetAdjust = 0;
    double offsetAdjustCount = 0;
    for (const auto& beat : beats) {
        double offset = fmod(beat, beatLength) - startOffset;
        if (abs(offset) < (kMaxSecsPhaseError * sampleRate)) {
            offsetAdjust += offset;
            offsetAdjustCount++;
        }
    }
    offsetAdjust /= offsetAdjustCount;
    qDebug() << "adjusting phase by" << offsetAdjust;
    DEBUG_ASSERT(abs(offsetAdjust) < (kMaxSecsPhaseError * sampleRate));

    return firstBeat - offsetAdjust;
}
