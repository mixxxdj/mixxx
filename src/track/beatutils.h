#pragma once

#include <QVector>

#include "audio/frame.h"
#include "audio/types.h"
#include "util/math.h"

class BeatUtils {
  public:
    struct ConstRegion {
        mixxx::audio::FramePos firstBeat;
        mixxx::audio::FrameDiff_t beatLength;
    };

    static double calculateBpm(const QVector<mixxx::audio::FramePos>& beats,
            mixxx::audio::SampleRate sampleRate);

    static QVector<ConstRegion> retrieveConstRegions(
            const QVector<mixxx::audio::FramePos>& coarseBeats,
            mixxx::audio::SampleRate sampleRate);

    static double calculateAverageBpm(int numberOfBeats,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::FramePos lowerFrame,
            mixxx::audio::FramePos upperFrame);

    static double makeConstBpm(
            const QVector<ConstRegion>& constantRegions,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::FramePos* pFirstBeat);

    static mixxx::audio::FramePos adjustPhase(
            mixxx::audio::FramePos firstBeat,
            double bpm,
            mixxx::audio::SampleRate sampleRate,
            const QVector<mixxx::audio::FramePos>& beats);

    static QVector<mixxx::audio::FramePos> getBeats(const QVector<ConstRegion>& constantRegions);

    static double roundBpmWithinRange(double minBpm, double centerBpm, double maxBpm);
};
