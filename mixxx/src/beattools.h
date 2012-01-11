/*
 * beattools.h
 *
 *  Created on: 30/nov/2011
 *      Author: vittorio
 */

#ifndef BEATTOOLS_H_
#define BEATTOOLS_H_

#include <QVector>

class BeatTools {
public:

    /*
     * This method detects the BPM given a set of beat positions.
     * We compute the average local BPM of by considering 8 beats
     * at a time. Internally, a sorted list of average BPM values is constructed
     * from which the statistical median is computed. This value provides
     * a pretty good guess of the global BPM value.
     */
    static double calculateBpm(QVector<double> beats, int SampleRate, int min_bpm, int max_bpm);
    static double findFirstCorrectBeat(QVector<double> rawBeats, int SampleRate, double global_bpm);

    /* This implement a method to find the best offset so that
     * the grid generated from bpm is close enough to the one we get from vamp.
     */
    static double calculateOffset(const QVector<double> beats1, const QVector<double> beats2, const int SampleRate, int min_bpm, int max_bpm);
};

#endif /* BEATTOOLS_H_ */
