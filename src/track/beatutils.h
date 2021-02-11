#pragma once

#include <QVector>

#include "audio/types.h"
#include "util/math.h"


class BeatUtils {
  public:
    struct ConstRegion {
        double firstBeat;
        double beatLength;
    };

    static double constrainBpm(double bpm, const int min_bpm,
                               const int max_bpm, bool aboveRange) {
        if (bpm <= 0.0 || min_bpm < 0 || max_bpm < 0 ||
            min_bpm >= max_bpm ||
            (bpm >= min_bpm && bpm <= max_bpm)) {
            return bpm;
        }

        if (isnan(bpm) || isinf(bpm)) {
            return 0.0;
        }

        if (!aboveRange) {
            while (bpm > max_bpm) {
                bpm /= 2.0;
            }
        }
        while (bpm < min_bpm) {
            bpm *= 2.0;
        }

        return bpm;
    }


    /*
     * This method detects the BPM given a set of beat positions.
     * We compute the average local BPM of by considering 8 beats
     * at a time. Internally, a sorted list of average BPM values is constructed
     * from which the statistical median is computed. This value provides
     * a pretty good guess of the global BPM value.
     */
    static double calculateBpm(const QVector<double>& beats, int SampleRate,
                               int min_bpm, int max_bpm);

    static QVector<ConstRegion> retrieveConstRegions(
            const QVector<double>& coarseBeats,
            const mixxx::audio::SampleRate& sampleRate);

    static double makeConstBpm(
            const QVector<ConstRegion>& constantRegions,
            int SampleRate,
            double* pFirstBeat);

    static double adjustPhase(
            double firstBeat,
            double bpm,
            int sampleRate,
            const QVector<double>& beats);

    static QVector<double> getBeats(const QVector<ConstRegion>& constantRegions);

  private:
    static double computeSampleMedian(const QList<double>& sortedItems);
    static double computeFilteredWeightedAverage(
            const QMap<double, int>& frequencyTable,
            const double filterCenter,
            const double filterTolerance,
            QMap<double, int>* filteredFrequencyTable);
    static QList<double> computeWindowedBpmsAndFrequencyHistogram(
            const QVector<double>& beats,
            const int windowSize,
            const int windowStep,
            const int sampleRate,
            QMap<double, int>* frequencyHistogram);
    static double roundBpmWithinRange(double minBpm, double centerBpm, double maxBpm);
};
