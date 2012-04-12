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
#define BPM_ERROR 0.05f //we are generous and assume the global_BPM to be at most 0.05 BPM far away from the correct one
#define N 12 //the raw beatgrid is divided into blocks of size N from which the local bpm is computed.
#include "beatutils.h"

static bool sDebug = false;
const double kBpmEpsilon = 0.2;

// Given a sorted set of numbers, find the sample median.
// http://en.wikipedia.org/wiki/Median#The_sample_median
double BeatUtils::computeSampleMedian(QList<double> sortedItems) {
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

double BeatUtils::computeFilteredWeightedAverage(
    const QMap<QString, int> frequencyTable,
    const double filterCenter,
    const double filterTolerance,
    QMap<QString, int>* filteredFrequencyTable) {
    double filterWeightedAverage = 0.0;
    int filterSum = 0;
    QMapIterator<QString, int> i(frequencyTable);

    while (i.hasNext()) {
        i.next();
        const double value = i.key().toDouble();
        const int frequency = i.value();

        if (fabs(value - filterCenter) <= filterTolerance) {
            // TODO(raffitea): Why > 1 ?
            if (i.value() > 1) {
                filterSum += frequency;
                filterWeightedAverage += value * frequency;
                filteredFrequencyTable->insert(i.key(), frequency);
                if(sDebug) {
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
        return 0;
    }
    return filterWeightedAverage / static_cast<double>(filterSum);
}

double BeatUtils::calculateBpm(QVector<double> beats, int SampleRate, int min_bpm, int max_bpm){
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
     QList<double> average_bpm_list;
     QMap<QString, int> frequency_table;

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
    int max_frequency = 0;
    double most_freq_bpm = 0;
    if (beats.size() < 2) {
        return 0;
    }
    double avg_bpm = 60.0 * beats.size() * SampleRate/(beats.last() - beats.first());
    for (int i = N; i < beats.size(); i += N) {
        //get start and end sample of the beats
        double start_sample = beats.at(i-N);
        double end_sample = beats.at(i);

        // Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample)/(SampleRate);
        avg_bpm = 60*N / time;

        if(avg_bpm < min_bpm)
        {
            avg_bpm *= 2;
        }
        if(avg_bpm > max_bpm){
            avg_bpm /=2;
        }

        // round BPM to have two decimal places
        double roundedBPM = floorf(avg_bpm * 100 + 0.5) / 100.0;

        // add to local BPM to list
        average_bpm_list << roundedBPM;

        QString rounded_bpm_str = QString::number(roundedBPM, 'g', 6);
        //qDebug() << "Local BPM: " << avg_bpm << " StringVal: " << local_bpm_str;
        if(frequency_table.contains(rounded_bpm_str)){
            int newFreq = frequency_table.value(rounded_bpm_str) + 1;
            //Set new Frequency
            frequency_table.insert(rounded_bpm_str, newFreq);

            if(newFreq > max_frequency){
                max_frequency = newFreq;
                most_freq_bpm = roundedBPM;
            }
        } else {
            frequency_table.insert(rounded_bpm_str, 1);
        }
        //qDebug() << "Average Bar BPM: " << avg_bpm;
    }

    if (average_bpm_list.empty()) {
        return avg_bpm;
    }

    // Get the median BPM.
    qSort(average_bpm_list);
    double median = computeSampleMedian(average_bpm_list);

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
    //qDebug() << "Max BPM Frequency=" << most_freq_bpm;

    // a subset of the 'frequency_table', where the bpm values are +-1 away from
    // the median average BPM.
    QMap<QString, int> filtered_bpm_frequency_table;
    const double kBpmFilterTolerance = 1.0;
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
     * Idea: Iterate over the original QM beat set where
     * some detected beats may be wrong:
     * The QM beat is considered as correct if:
     *  - the beat position can be approached by a beat grid obtained by the global BPM
     *
     * If the beat turns out correct, we can compute the error in BPM units.
     * E.g., we can check the QM beat position after 60 seconds. Ideally,
     * the approached beat is just a couple of samples away, i.e., not worse than 0.05 BPM units.
     * The distance between these two samples can be used for BPM error correction.
     */

     double perfect_bpm = 0;
     double firstCorrectBeatSample = beats.first();
     bool foundFirstCorrectBeat = false;
     double global_bpm = filterWeightedAverageBpm;

     int counter = 0;
     int perfectBeats = 0;
     for (int i = N; i < beats.size(); i += N) {
         // get start and end sample of the beats
         double beat_start = beats.at(i-N);
         double beat_end = beats.at(i);

         // Time needed to count a bar (N beats)
         double time = (beat_end - beat_start)/SampleRate;
         double local_bpm = 60.0 * N / time;
         // round BPM to have two decimal places
         local_bpm = floorf(local_bpm * 100 + 0.5) / 100;

         //qDebug() << "Local BPM beat " << i << ": " << local_bpm;
         QString local_bpm_str = QString::number(local_bpm,'g',6);
         if (filtered_bpm_frequency_table.contains(local_bpm_str) &&
             fabs(local_bpm - median) < BPM_ERROR) {
             if (!foundFirstCorrectBeat) {
                firstCorrectBeatSample = beat_start;
                foundFirstCorrectBeat = true;
                if (sDebug) {
                    qDebug() << "Beat #" << (i-N)
                             << "is considered as reference beat with BPM:"
                             << local_bpm;
                }
            }
         }
         if (foundFirstCorrectBeat) {
              counter += N;
              double time2 = (beat_end - firstCorrectBeatSample)/SampleRate;
              double correctedBpm = 60 * counter / time2;

              if (fabs(correctedBpm - global_bpm) <= BPM_ERROR) {
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

     if (perfectBeats > 0) {
         global_bpm = perfect_bpm / perfectBeats;
     }

     // last guess to make BPM more accurate: rounding values like 127.96 or 128.01 to 128.0
     double rounded_bpm = floor(global_bpm + 0.5);
     double bpm_diff = fabs(rounded_bpm - global_bpm);
     if (sDebug) {
         qDebug() << "Perfect BPM=" << global_bpm;
         qDebug() << "Rounded Perfect BPM=" << rounded_bpm;
         qDebug() << "Rounded difference=" << bpm_diff;
         qDebug() << "Perform rounding=" << (bpm_diff <= BPM_ERROR);
     }

     return fabs(bpm_diff) <= BPM_ERROR ? rounded_bpm : global_bpm;
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
    const double beatLength1Epsilon = beatlength1 * 0.2;

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
                                       int SampleRate, double global_bpm) {
    // TODO(rryan) I'm not sure this is the best way to do this. Because you are
    // using a window of N, if the first "correct" beat is at N-1 then you'll
    // ignore it. It might be better (though more CPU intensive) to slide the
    // window forward by 1 instead of N.
    for (int i = N; i < rawbeats.size(); i += N) {
        // get start and end sample of the beats
        double start_sample = rawbeats.at(i-N);
        double end_sample = rawbeats.at(i);

        // The time in seconds represented by this sample range.
        double time = (end_sample - start_sample)/(SampleRate);

        // Average BPM within this sample range.
        double avg_bpm = 60.0 * N / time;

        //qDebug() << "Local BPM between beat " << (i-N) << " and " << i << " is " << avg_bpm;

        // If the local BPM is within kBpmEpsilon of the global BPM then use
        // this window as the first beat.
        if (fabs(global_bpm - avg_bpm) <= kBpmEpsilon) {
            //qDebug() << "Using beat " << (i-N) << " as first beat";
            return start_sample;
        }
    }

    // If we didn't find any beat that matched the window, return the first
    // beat.
    if (rawbeats.size() > 0) {
        return rawbeats.at(0);
    }
    return 0.0f;
}

// static
QVector<double> BeatUtils::calculateFixedTempoBeats(
    bool enableOffsetCorrection,
    const QVector<double> rawbeats, const int sampleRate,
    const int totalSamples, const double globalBpm,
    const int minBpm, const int maxBpm) {
    /*
     * By default Vamp does not assume a 4/4 signature.
     * This is basically a good property of Vamp, however,
     * it leads to inaccurate beat grids if a 4/4 signature is given.
     * What is the problem? Almost all modern dance music from the last decades
     * refer to 4/4 signatures. Thus, we must 'correct' the beat positions of Vamp
     */

    QVector <double> corrbeats;
    // Length of a beat at m_dBpm in mono samples.
    double beat_length = (60.0 * sampleRate / globalBpm);
    double firstCorrectBeat =
            BeatUtils::findFirstCorrectBeat(rawbeats, sampleRate, globalBpm);

    // We start building a fixed beat grid from m_dBpm and the first beat from
    // rawbeats that matches m_dBpm.
    double i = firstCorrectBeat;
    while (i <= totalSamples) {
        corrbeats << i;
        i += beat_length;
    }

    if (rawbeats.size() == 1 || corrbeats.size()==1) {
        return corrbeats;
    }

    /*
     * BeatUtils::calculateOffset compares the beats from Vamp and the beats from
     * the beat grid constructed above. See beatutils.cpp for details.
     */
    double offset = 0;
    if (enableOffsetCorrection) {
        qDebug() << "Calculating best offset";
        offset = BeatUtils::calculateOffset(rawbeats, globalBpm,
                                            corrbeats, sampleRate);
    }

    double FirstFrame = offset + firstCorrectBeat;
    while (FirstFrame < 0) {
        FirstFrame += beat_length;
    }
    while (FirstFrame > beat_length) {
        FirstFrame -= beat_length;
    }

    i = floor(FirstFrame + 0.5);

    if (sDebug) {
        qDebug() << "First Frame is at " << i;
        qDebug() << "It was at " << rawbeats.at(0);
    }

    corrbeats.clear();
    while (i < totalSamples) {
        corrbeats << i;
        i += beat_length;
    }

    return corrbeats;
}
