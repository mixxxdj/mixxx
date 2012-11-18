/*
 * beatutils.cpp
 *
 *  Created on: 30/nov/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <math.h>

#include "track/beatutils.h"

// we are generous and assume the global_BPM to be at most 0.05 BPM far away
// from the correct one
#define BPM_ERROR 0.05f

// the raw beatgrid is divided into blocks of size N from which the local bpm is
// computed.
#define N 12

static bool sDebug = false;

const double kCorrectBeatLocalBpmEpsilon = 0.05; //0.2;
const int kHistogramDecimalPlaces = 2;
const double kHistogramDecimalScale = pow(10.0, kHistogramDecimalPlaces);
const double kBpmFilterTolerance = 1.0;

void BeatUtils::printBeatStatistics(const QVector<double>& beats, int SampleRate) {
    if (!sDebug) {
        return;
    }
    QMap<double, int> frequency;

    for (int i = N; i < beats.size(); i += 1) {
        double beat_start = beats.at(i-N);
        double beat_end = beats.at(i);

        // Time needed to count a bar (N beats)
        const double time = (beat_end - beat_start)/SampleRate;
        double local_bpm = 60.0 * N / time;

        qDebug() << "Beat" << i << "local BPM:" << local_bpm;

        local_bpm = floorf(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;
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
        int item_position = sortedItems.size()/2;
        double item_value1 = sortedItems.at(item_position);
        double item_value2 = sortedItems.at(item_position + 1);
        return (item_value1 + item_value2)/2.0;
    }

    // When there are an odd number of elements, find the {(n+1)/2}th item in
    // the sorted list.
    int item_position = (sortedItems.size() + 1)/2;
    return sortedItems.at(item_position - 1);
}

QList<double> BeatUtils::computeWindowedBpmsAndFrequencyHistogram(
        const QVector<double> beats, const int windowSize, const int windowStep,
        const int sampleRate, QMap<double, int>* frequencyHistogram) {
    QList<double> averageBpmList;
    for (int i = windowSize; i < beats.size(); i += windowStep) {
        //get start and end sample of the beats
        double start_sample = beats.at(i - windowSize);
        double end_sample = beats.at(i);

        // Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample)/sampleRate;
        double localBpm = 60.0 * windowSize / time;

        // round BPM to have two decimal places
        double roundedBpm = floorf(localBpm * kHistogramDecimalScale + 0.5) /
                kHistogramDecimalScale;

        // add to local BPM to list and increment frequency count
        averageBpmList << roundedBpm;
        (*frequencyHistogram)[roundedBpm] += 1;
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

double BeatUtils::calculateBpm(const QVector<double>& beats, int SampleRate, int min_bpm, int max_bpm){
    /*
     * Let's compute the average local
     * BPM for N subsequent beats.
     * The average BPMs are
     * added to a list from which the statistical
     * median is computed
     *
     * N=8 seems to work great; We coincide with Traktor's
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
     * 4 beats varies frequently by +-2 BPM.  Somtimes there N subsequent beats
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
        return 60.0 * beats.size() * SampleRate / (beats.last() - beats.first());
    }

    QMap<double, int> frequency_table;
    QList<double> average_bpm_list = computeWindowedBpmsAndFrequencyHistogram(
        beats, N, 1, SampleRate, &frequency_table);

    // Get the median BPM.
    qSort(average_bpm_list);
    const double median = computeSampleMedian(average_bpm_list);

    /*
     * Okay, let's consider the median an estimation of the BPM To not soley
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
         double time = (beat_end - beat_start)/SampleRate;
         double local_bpm = 60.0 * N / time;
         // round BPM to have two decimal places
         local_bpm = floorf(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;

         //qDebug() << "Local BPM beat " << i << ": " << local_bpm;
         if (!foundFirstCorrectBeat &&
             filtered_bpm_frequency_table.contains(local_bpm) &&
             fabs(local_bpm - filterWeightedAverageBpm) < BPM_ERROR) {
             firstCorrectBeatSample = beat_start;
             foundFirstCorrectBeat = true;
             if (sDebug) {
                 qDebug() << "Beat #" << (i-N)
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
             double time2 = (beat_end - firstCorrectBeatSample)/SampleRate;
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

double BeatUtils::calculateOffset(
    const QVector<double> beats1, const double bpm1,
    const QVector<double> beats2, const int SampleRate) {
    /*
     * Here we compare to beats vector and try to determine the best offset
     * based on the occurences, i.e. by assuming that the almost correct beats
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
            QVector<double>::const_iterator it = qUpperBound(
                beats1.begin(), beats1.end(), beats2_beat);
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
    for (int i = N; i < rawbeats.size(); i++) {
        // get start and end sample of the beats
        double start_sample = rawbeats.at(i-N);
        double end_sample = rawbeats.at(i);

        // The time in seconds represented by this sample range.
        double time = (end_sample - start_sample)/SampleRate;

        // Average BPM within this sample range.
        double avg_bpm = 60.0 * N / time;

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
    return !rawbeats.empty() ? rawbeats.first() : 0.0f;
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
     * calculateOffset compares the beats from the analyser and the
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
