// Created on: 30/nov/2011
// Author: vittorio

#ifndef BEATUTILS_H_
#define BEATUTILS_H_

#include <QVector>

class BeatUtils {
  public:
    /*
     * This method detects the BPM given a set of beat positions.
     * We compute the average local BPM of by considering 8 beats
     * at a time. Internally, a sorted list of average BPM values is constructed
     * from which the statistical median is computed. This value provides
     * a pretty good guess of the global BPM value.
     */
    static double calculateBpm(QVector<double> beats, int SampleRate,
                               int min_bpm, int max_bpm);
    static double findFirstCorrectBeat(const QVector<double> rawBeats,
                                       int SampleRate, double global_bpm);

    /* This implement a method to find the best offset so that
     * the grid generated from bpm is close enough to the one we get from vamp.
     */
    static double calculateOffset(
        const QVector<double> beats1, const double bpm1,
        const QVector<double> beats2, const int SampleRate);

    // By default Vamp does not assume a 4/4 signature. This is basically a good
    // property of Vamp, however, it leads to inaccurate beat grids if a 4/4
    // signature is given.  What is the problem? Almost all modern dance music
    // from the last decades refer to 4/4 signatures. Given a set of beat frame
    // positions, this method calculates the position of the first beat assuming
    // the beats have a fixed tempo given by globalBpm.
    static double calculateFixedTempoFirstBeat(
        bool enableOffsetCorrection,
        const QVector<double> rawbeats, const int sampleRate,
        const int totalSamples, const double globalBpm);

  private:
    static double computeSampleMedian(QList<double> sortedItems);
    static double computeFilteredWeightedAverage(
        const QMap<QString, int> frequencyTable,
        const double filterCenter,
        const double filterTolerance,
        QMap<QString, int>* filteredFrequencyTable);

};

#endif /* BEATUTILS_H_ */
