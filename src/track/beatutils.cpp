/*
 * beatutils.cpp
 *
 *  Created on: 30/nov/2011
 *      Author: vittorio
 */

#include "track/beatutils.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QtDebug>
#include <algorithm>

#include "util/assert.h"
#include "util/math.h"
#include "util/descriptivestatistics.h"
#include "util/windowedstatistics.h"

namespace {

// we are generous and assume the global_BPM to be at most 0.05 BPM far away
// from the correct one
constexpr double kMaxBpmError = 0.05;
// When ironing the grid for long sequences of const tempo we use
// a 25ms tolerence because this small of a difference is inaudible
constexpr double kMaxSecsPhaseError = 0.025;

// the raw beatgrid is divided into blocks of size N from which the local bpm is
// computed. Tweaked from 8 to 12 which improves the BPM accuracy for 'problem songs'.
constexpr int kBeatsToCountTempo = 12;

constexpr int kMaxBeatsForChange = 32;

static bool sDebug = false;

const double kCorrectBeatLocalBpmEpsilon = 0.05; //0.2;
const int kHistogramDecimalPlaces = 2;
const double kHistogramDecimalScale = pow(10.0, kHistogramDecimalPlaces);
const double kBpmFilterTolerance = 1.0;
// maximum difference for BPMs to be considered the same
constexpr double kMaxDiffSameBpm = 0.3;

QVector<double> makeQVector(QVector<double>::iterator begin, QVector<double>::iterator end) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QVector<double>(begin, end);
#else
    return QVector<double>::fromStdVector(std::vector<double>(begin, end));
#endif
}

} // namespace


QMap<int, double> BeatUtils::findStableTempoRegions(const QVector<double>& beats,
        const mixxx::audio::SampleRate& sampleRate) {
    
    QVector<double> durationOfBeats = computeDurationOfEachBeat(beats);
    auto sortedDurationOfBeats = durationOfBeats.toList();
    std::sort(sortedDurationOfBeats.begin(), sortedDurationOfBeats.end());
    // We have to make sure we have odd numbers here because we are going
    // to compute the median value and it must belong to frequencyOfTempos.
    // The median is only used as rough guess of the tempo so it's ok that
    // we accidentally change the median by popping the last value.
    bool poped = false;
    double reserveLast = 0.0;
    if (sortedDurationOfBeats.size() % 2 == 0 && sortedDurationOfBeats.size() > 1) {
        reserveLast = sortedDurationOfBeats.last();
        sortedDurationOfBeats.pop_back();
        poped = true;
    }
    const double medianBeatLenght = computeSampleMedian(sortedDurationOfBeats);
    if (poped) {
        sortedDurationOfBeats << reserveLast;
    }
    // Forming a meter perception takes a few seconds, so we assume sections of consistent
    // metrical structure to be at least around 10s long. So we use a window of the double
    // of that in our filtering since changes in the tempo shorter than that are very likely 
    // to be noise from the analyzer. The median filter is good to discard the outliers values
    // but still may fluctatue because of the jitter so we still look for the most frequent median
    int numberOfBeatsInFilteringWindow = (10 / (medianBeatLenght / sampleRate)) * 2;
    if (numberOfBeatsInFilteringWindow % 2 == 0) {
        numberOfBeatsInFilteringWindow += 1;
    }
    auto tempoMedianFilter = MovingMedian(numberOfBeatsInFilteringWindow);
    auto mostFrequentTempo = MovingMode(numberOfBeatsInFilteringWindow, 1.0);
    int currentBeat = -1;
    int lastBeatChange = 0;
    QMap<int, double> stableBeatLenghtByPosition;
    // We start at beat 0 with the median bpm as rough guess of the "correct" tempo
    stableBeatLenghtByPosition.insert(lastBeatChange, medianBeatLenght);
    // Here we are going to track the significant tempo changes over the track to make
    // regions of stable tempos and independently make const tempo or ironed grid for them
    for (double beatLenght : durationOfBeats) {
        currentBeat += 1;
        double filteredBeatLenght = tempoMedianFilter.pushAndEvaluate(beatLenght);
        double newStableBeatLenght = mostFrequentTempo.pushAndEvaluate(filteredBeatLenght);
        // The analyzer has some jitter that causes a steady beat to fluctuate around the correct
        // value so we don't consider changes to a neighboring value in the ordered tempo table
        if (newStableBeatLenght == stableBeatLenghtByPosition.last()) {
            continue;
        // Check if the new tempo is the right neighbor of the previous tempo
        } else if (stableBeatLenghtByPosition.last() != sortedDurationOfBeats.last() &&
                newStableBeatLenght == *(std::upper_bound(
                        sortedDurationOfBeats.begin(), 
                        sortedDurationOfBeats.end(), stableBeatLenghtByPosition.last()) + 1)) {
            continue;
        // Check if the new tempo is the left neighbor of the previous tempo
        } else if (stableBeatLenghtByPosition.last() != sortedDurationOfBeats.first() &&
                newStableBeatLenght == *(std::lower_bound(
                        sortedDurationOfBeats.begin(),
                        sortedDurationOfBeats.end(), stableBeatLenghtByPosition.last()) - 1)) {
            continue;
        } else {
            // We need to make sure newStableBeatLenght belongs to sortedDurationOfBeats
            // because in the loop we call std::lower_bound on it and will dereference
            if (std::binary_search(sortedDurationOfBeats.begin(), sortedDurationOfBeats.end(), newStableBeatLenght)) {
                lastBeatChange = currentBeat - tempoMedianFilter.lag() - mostFrequentTempo.lag();
                // Since we used the median as guess of the first tempo
                // we change position at 0 if we detect a change in very beginning 
                if (lastBeatChange > kBeatsToCountTempo) {
                    stableBeatLenghtByPosition.insert(lastBeatChange, newStableBeatLenght);
                } else {
                    stableBeatLenghtByPosition.insert(0, newStableBeatLenght);
                }
            }
        }
    }
    // We also add the median as rough guess as our last tempo
    // This way we always have both sentinels for the whole track
    // as a constant tempo region if no significant tempo changes
    stableBeatLenghtByPosition.insert(durationOfBeats.count(), medianBeatLenght);
    return stableBeatLenghtByPosition;
}

void BeatUtils::removeSmallArrhythmic(
        QVector<double>& beats, const mixxx::audio::SampleRate& sampleRate, const QMap<int, double>& stableBeatLenghtByPosition) {
    // A common problem the analyzer has is to detect arrhythmic regions
    // of tacks with a constant tempo as in a different unsteady tempo.
    // This happens frequently on builds and breaks with heavy effects on edm music.
    // Since these occurs most on beatless regions we do not want them to be
    // on a different tempo, because they are still syncable in the true tempo
    // We arbitraly remove these arrhythmic regions if they are short than 32 beats
    // and return to same tempo they start deviating from.
    // TODO(Cristiano) Use a better heuristic for "finding" these regions
    // like for example the avarage energy of the beats
    auto positionsWithTempoChange = stableBeatLenghtByPosition.keys();
    auto beatLenghts = stableBeatLenghtByPosition.values();
    // We are going to make a deep copy since we are very likely to
    // shift our whole beats vector which will results in a copy anyway
    QVector<double> anchoredBeats;
    anchoredBeats.reserve(beats.size());
    anchoredBeats << makeQVector(beats.begin(), beats.begin() + positionsWithTempoChange[1] + 1);

    for (int i = 2; i < positionsWithTempoChange.size(); i += 1) {
        double previousTempoRoughGuess = 60.0 * sampleRate / beatLenghts[i - 2];
        int limitAtLeft = positionsWithTempoChange[i - 1];
        int limitAtRight = positionsWithTempoChange[i];
        int lenghtOfChange = limitAtRight - limitAtLeft;
        double nextTempoRoughGuess = 60.0 * sampleRate / beatLenghts[i];
        if (lenghtOfChange <= kMaxBeatsForChange &&
                fabs(nextTempoRoughGuess - previousTempoRoughGuess) < kMaxDiffSameBpm) {
            //qDebug() << "removing" << limitAtRight - limitAtLeft << "beats from" << limitAtLeft << limitAtRight;
            auto beatsAtLeft = makeQVector(
                    beats.begin() + positionsWithTempoChange[i - 2],
                    beats.begin() + positionsWithTempoChange[i - 1]);
            double tempoAtLeft = calculateBpm(beatsAtLeft, sampleRate, 60, 180);
            double beatLength = (60.0 * sampleRate) / tempoAtLeft;
            double regionLengthInFrames = beats[limitAtRight] - beats[limitAtLeft];
            int beatsToAdd = floor((regionLengthInFrames / beatLength) + 0.5);
            // Make sure the last beat we add is extacly at litmitAtRight
            double preciseBeatLeght = regionLengthInFrames / beatsToAdd;
            double beatOffset = beats[limitAtLeft];
            int beatsAdded = 0;
            while (beatsAdded != beatsToAdd) {
                anchoredBeats << floor(beatOffset + 0.5);
                beatOffset += preciseBeatLeght;
                beatsAdded += 1;
            }
        } else {
            anchoredBeats << makeQVector(
                    beats.begin() + positionsWithTempoChange[i - 1],
                    beats.begin() + positionsWithTempoChange[i] + 1);
        }
    }
    double previousBeat = 0.0;
    beats.clear();
    for (double& beat : anchoredBeats) {
        if (beat - previousBeat > 0.01) {
            beats << beat;
            previousBeat = beat;
        }
    }
}

QVector<double> BeatUtils::correctBeatmap(
        QVector<double>& rawBeats, const mixxx::audio::SampleRate& sampleRate, bool removeArrythmic) {
    QMap<int, double> stableBeatLenghtByPosition = findStableTempoRegions(rawBeats, sampleRate);
    if (removeArrythmic) {
        removeSmallArrhythmic(rawBeats, sampleRate, stableBeatLenghtByPosition);
        // Since we changed our beats vector we need to recalculate its tempos
        // TODO(Cristiano) adjust these on the fly on removeSmallArrhythmic
        stableBeatLenghtByPosition = findStableTempoRegions(rawBeats, sampleRate);
    }
    QVector<double> correctedBeats;
    QVector<double> ironedBeats;
    correctedBeats.reserve(rawBeats.size());
    auto tempoChanges = stableBeatLenghtByPosition.keys();
    for (int lastTempoChage = 1;
            lastTempoChage < tempoChanges.size();
            lastTempoChage++) {
        int beatStart = tempoChanges[lastTempoChage - 1];
        int beatEnd = tempoChanges[lastTempoChage];
        
        auto splittedAtTempoChange = makeQVector(
                rawBeats.begin() + beatStart, rawBeats.begin() + beatEnd + 1);
        ironedBeats = calculateIronedGrid(splittedAtTempoChange, sampleRate);

        if (correctedBeats.size() > 0) {
            if (correctedBeats.last() == ironedBeats.first()) {
                correctedBeats.pop_back();
            }
        }
        correctedBeats << ironedBeats;
    }
    return correctedBeats;
}

QVector<double> BeatUtils::calculateIronedGrid(
        const QVector<double>& rawbeats, const mixxx::audio::SampleRate& sampleRate) {
    // Daniel's ironing algorithm
    // loop backwards through the raw beats. and calculate the average beat length from the first beat.
    // add an inner loop and check for outliers using the momentary average as beat length.
    // once you have found an average with only single outliers, store the beats using the current average.
    // reset and do the loop again, starting with the region from the found beat to the end.
    double maxPhaseError = kMaxSecsPhaseError * sampleRate;
    int leftIndex = 0;
    int rightIndex = rawbeats.size() - 1;
    QVector<double> ironedBeats;
    while (leftIndex < rawbeats.size() - 1) {
        double meanBeatLength = (rawbeats[rightIndex] - rawbeats[leftIndex]) / (rightIndex - leftIndex);
        int outliersCount = 0;
        double beatOffset = rawbeats[leftIndex];
        for (int i = leftIndex; i < rightIndex; i += 1) {
            double phaseError = beatOffset - rawbeats[i];
            if (fabs(phaseError) > maxPhaseError) {
                outliersCount += 1;
            }
            beatOffset += meanBeatLength;
        }
        if (outliersCount <= 1) {
            beatOffset = rawbeats[leftIndex];
            for (int i = leftIndex; i < rightIndex; i += 1) {
                ironedBeats << floor(beatOffset + 0.5);
                beatOffset += meanBeatLength;
            }
            leftIndex = rightIndex;
            rightIndex = rawbeats.size() - 1;
            continue;
        }
        rightIndex -= 1;
    }
    return ironedBeats;
}

void BeatUtils::printBeatStatistics(const QVector<double>& beats, int SampleRate) {
    if (!sDebug) {
        return;
    }
    QMap<double, int> frequency;

    for (int i = kBeatsToCountTempo; i < beats.size(); i += 1) {
        double beat_start = beats.at(i - kBeatsToCountTempo);
        double beat_end = beats.at(i);

        // Time needed to count a bar (N beats)
        const double time = (beat_end - beat_start) / SampleRate;
        if (time == 0) continue;
        double local_bpm = 60.0 * kBeatsToCountTempo / time;

        qDebug() << "Beat" << i << "local BPM:" << local_bpm;

        local_bpm = floor(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;
        frequency[local_bpm] += 1;
    }

    qDebug() << "Rounded local BPM histogram:";

    QMapIterator<double, int> it(frequency);
    while (it.hasNext()) {
        it.next();
        qDebug() << it.key() << ":" << it.value();
    }
}

// Given a sorted set of numbers, find the sample median.
// http://en.wikipedia.org/wiki/Median#The_sample_median
double BeatUtils::computeSampleMedian(QList<double> sortedItems) {
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

QVector<double> BeatUtils::computeDurationOfEachBeat(const QVector<double>& beats) {
    QVector<double> beatDurations;
    beatDurations.reserve(beats.size() -1);
    auto previousBeat = beats.begin();
    for(auto beat = beats.begin() + 1; beat != beats.end(); beat += 1) {
        double duration = *beat - *previousBeat;
        beatDurations << duration;
        previousBeat = beat;
    }
    return beatDurations;
}

QList<double> BeatUtils::computeWindowedBpmsAndFrequencyHistogram(
        const QVector<double> beats, int windowSize, const int windowStep,
        const int sampleRate, QMap<double, int>* pFrequencyHistogram) {
    DEBUG_ASSERT(pFrequencyHistogram);
    // avoid out of range access case beats with small
    while (beats.size() - 1 < windowSize * 2) {
        windowSize /= 2;
    }
    QList<double> averageBpmList;
    for (int i = 0; i < beats.size(); i += windowStep) {
        //get start and end sample of the beats
        double start_sample = beats.at(i);
        double end_sample;
        if (i + windowSize > beats.size() - 1) {
            end_sample = beats.at(i);
            start_sample = beats.at(i - windowSize);
        } else {
            end_sample = beats.at(i + windowSize);
        }
        // Time needed to count kbeats
        double time = (end_sample - start_sample) / sampleRate;
        if (time == 0) continue;
        double localBpm = 60.0 * windowSize / time;
        // round BPM to have two decimal places
        double roundedBpm = floor(localBpm * kHistogramDecimalScale + 0.5) /
                kHistogramDecimalScale;
        // add to local BPM to list and increment frequency count
        averageBpmList << roundedBpm;
        (*pFrequencyHistogram)[roundedBpm] += 1;
    }
    return averageBpmList;
}

double BeatUtils::computeFilteredWeightedAverage(
    const QMap<double, int> frequencyTable,
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

double BeatUtils::calculateBpm(const QVector<double>& beats, int SampleRate,
                               int min_bpm, int max_bpm) {
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
    if (beats.size() <= kBeatsToCountTempo) {
        return 60.0 * (beats.size()-1) * SampleRate / (beats.last() - beats.first());
    }

    QMap<double, int> frequency_table;
    QList<double> average_bpm_list = computeWindowedBpmsAndFrequencyHistogram(
        beats, kBeatsToCountTempo, 1, SampleRate, &frequency_table);

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
     for (int i = kBeatsToCountTempo; i < beats.size(); i += 1) {
         // get start and end sample of the beats
         double beat_start = beats.at(i-kBeatsToCountTempo);
         double beat_end = beats.at(i);

         // Time needed to count a bar (N beats)
         double time = (beat_end - beat_start) / SampleRate;
         if (time == 0) continue;
         double local_bpm = 60.0 * kBeatsToCountTempo / time;
         // round BPM to have two decimal places
         local_bpm = floor(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;

         //qDebug() << "Local BPM beat " << i << ": " << local_bpm;
         if (!foundFirstCorrectBeat &&
                 filtered_bpm_frequency_table.contains(local_bpm) &&
                 fabs(local_bpm - filterWeightedAverageBpm) < kMaxBpmError) {
             firstCorrectBeatSample = beat_start;
             foundFirstCorrectBeat = true;
             if (sDebug) {
                 qDebug() << "Beat #" << (i - kBeatsToCountTempo)
                          << "is considered as reference beat with BPM:"
                          << local_bpm;
             }
         }
         if (foundFirstCorrectBeat) {
             if (counter == 0) {
                 counter = kBeatsToCountTempo;
             } else {
                 counter += 1;
             }
             double time2 = (beat_end - firstCorrectBeatSample) / SampleRate;
             double correctedBpm = 60 * counter / time2;

             if (fabs(correctedBpm - filterWeightedAverageBpm) <= kMaxBpmError) {
                 perfect_bpm += correctedBpm;
                 ++perfectBeats;
                 if (sDebug) {
                     qDebug() << "Beat #" << (i-kBeatsToCountTempo)
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
     bool perform_rounding = (bpm_diff <= kMaxBpmError);

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

double BeatUtils::calculateOffset(
    const QVector<double> beats1, const double bpm1,
    const QVector<double> beats2, const int SampleRate) {
    /*
     * Here we compare to beats vector and try to determine the best offset
     * based on the occurrences, i.e. by assuming that the almost correct beats
     * are more than the "false" ones.
     */
    const double beatlength1 = (60.0 * SampleRate / bpm1);
    const double beatLength1Epsilon = beatlength1 * 0.02;

    int bestFreq = 1;
    double bestOffset = beats1.at(0) - beats2.at(0);

    // Sweep offset from [-beatlength1/2, beatlength1/2]
    double offset = floor(-beatlength1 / 2);
    while (offset < (beatlength1 / 2)) {
        int freq = 0;
        for (int i = 0; i < beats2.size(); i += 4) {
            double beats2_beat = beats2.at(i);
            QVector<double>::const_iterator it = std::upper_bound(
                beats1.constBegin(), beats1.constEnd(), beats2_beat);
            if (fabs(*it - beats2_beat - offset) <= beatLength1Epsilon) {
                freq++;
            }
        }
        if (freq > bestFreq) {
            bestFreq = freq;
            bestOffset = offset;
        }
        offset++;
    }

    if (sDebug) {
        qDebug() << "Best offset " << bestOffset << "guarantees that"
                << bestFreq << "over" << beats1.size()/4
                << "beats almost coincides.";
    }

    return floor(bestOffset + beatLength1Epsilon);
}

double BeatUtils::findFirstCorrectBeat(const QVector<double> rawbeats,
                                       const int SampleRate, const double global_bpm) {
    for (int i = kBeatsToCountTempo; i < rawbeats.size(); i++) {
        // get start and end sample of the beats
        double start_sample = rawbeats.at(i-kBeatsToCountTempo);
        double end_sample = rawbeats.at(i);

        // The time in seconds represented by this sample range.
        double time = (end_sample - start_sample)/SampleRate;

        // Average BPM within this sample range.
        double avg_bpm = 60.0 * kBeatsToCountTempo / time;

        //qDebug() << "Local BPM between beat " << (i-N) << " and " << i << " is " << avg_bpm;

        // If the local BPM is within kCorrectBeatLocalBpmEpsilon of the global
        // BPM then use this window as the first beat.
        if (fabs(global_bpm - avg_bpm) <= kCorrectBeatLocalBpmEpsilon) {
            //qDebug() << "Using beat " << (i-N) << " as first beat";
            return start_sample;
        }
    }

    // If we didn't find any beat that matched the window, return the first
    // beat.
    return !rawbeats.empty() ? rawbeats.first() : 0.0;
}

// static
double BeatUtils::calculateFixedTempoFirstBeat(
    bool enableOffsetCorrection,
    const QVector<double> rawbeats, const int sampleRate,
    const int totalSamples, const double globalBpm) {
    if (rawbeats.size() == 0) {
        return 0;
    }

    if (!enableOffsetCorrection) {
        return rawbeats.first();
    }

    QVector <double> corrbeats;
    // Length of a beat at globalBpm in mono samples.
    const double beat_length = 60.0 * sampleRate / globalBpm;

    double firstCorrectBeat = findFirstCorrectBeat(
        rawbeats, sampleRate, globalBpm);

    // We start building a fixed beat grid at globalBpm and the first beat from
    // rawbeats that matches globalBpm.
    double i = firstCorrectBeat;
    while (i <= totalSamples) {
        corrbeats << i;
        i += beat_length;
    }

    if (rawbeats.size() == 1 || corrbeats.size()==1) {
        return firstCorrectBeat;
    }

    /*
     * calculateOffset compares the beats from the analyzer and the
     * beats from the beat grid constructed above in corrbeats.
     */
    // qDebug() << "Calculating best offset";
    // double offset = calculateOffset(rawbeats, globalBpm, corrbeats, sampleRate);
    // // Adjust firstCorrectBeat by offset
    // firstCorrectBeat += offset;


    // Find the smallest positive beat that is linked to firstCorrectBeat by
    // beat_length steps.
    double FirstFrame = firstCorrectBeat;
    while (FirstFrame < 0) {
        FirstFrame += beat_length;
    }
    while (FirstFrame > beat_length) {
        FirstFrame -= beat_length;
    }

    // Round to nearest integer.
    double firstBeat = floor(FirstFrame + 0.5);
    if (sDebug) {
        qDebug() << "calculateFixedTempoFirstBeat chose a first beat at frame" << firstBeat
                 << "while the first raw beat was at" << rawbeats.at(0);
    }
    return firstBeat;
}
