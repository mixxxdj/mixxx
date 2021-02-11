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
            const mixxx::audio::SampleRate& sampleRate);

    static QVector<ConstRegion> retrieveConstRegions(
            const QVector<double>& coarseBeats,
            const mixxx::audio::SampleRate& sampleRate);

    static double makeConstBpm(
            const QVector<ConstRegion>& constantRegions,
            const mixxx::audio::SampleRate& sampleRate,
            double* pFirstBeat);

    static double adjustPhase(
            double firstBeat,
            double bpm,
            const mixxx::audio::SampleRate& sampleRate,
            const QVector<double>& beats);

    static QVector<double> getBeats(const QVector<ConstRegion>& constantRegions);

  private:
    static double roundBpmWithinRange(double minBpm, double centerBpm, double maxBpm);
};
