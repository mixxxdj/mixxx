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

    static double calculateBpm(const QVector<double>& beats,
            mixxx::audio::SampleRate sampleRate);

    static QVector<ConstRegion> retrieveConstRegions(
            const QVector<double>& coarseBeats,
            mixxx::audio::SampleRate sampleRate);

    static double calculateAverageBpm(int numberOfBeats,
            mixxx::audio::SampleRate sampleRate,
            double lowerFrame,
            double upperFrame);

    static double makeConstBpm(
            const QVector<ConstRegion>& constantRegions,
            mixxx::audio::SampleRate sampleRate,
            double* pFirstBeat);

    static double adjustPhase(
            double firstBeat,
            double bpm,
            mixxx::audio::SampleRate sampleRate,
            const QVector<double>& beats);

    static QVector<double> getBeats(const QVector<ConstRegion>& constantRegions);

  private:
    static double roundBpmWithinRange(double minBpm, double centerBpm, double maxBpm);
};
