#pragma once

#include <QHash>

#include "track/beats.h"
#include "track/track.h"

class BeatFactory {
  public:
    static mixxx::BeatsInternal loadBeatsFromByteArray(const TrackPointer& track,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);
    static mixxx::BeatsPointer makeBeatGrid(const TrackPointer& track,
            double dBpm,
            double dFirstBeatSample);

    static QString getPreferredVersion(const bool bEnableFixedTempoCorrection);

    static QString getPreferredSubVersion(
            const bool bEnableFixedTempoCorrection,
            const bool bEnableOffsetCorrection,
            const int iMinBpm,
            const int iMaxBpm,
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsInternal makePreferredBeats(const TrackPointer& track,
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            const bool bEnableFixedTempoCorrection,
            const bool bEnableOffsetCorrection,
            const int iTotalSamples,
            const int iMinBpm,
            const int iMaxBpm);
};
