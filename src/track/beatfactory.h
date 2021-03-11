#pragma once

#include <QHash>

#include "track/beats.h"

class Track;

class BeatFactory {
  public:
    static mixxx::BeatsPointer loadBeatsFromByteArray(
            mixxx::audio::SampleRate sampleRate,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);
    static mixxx::BeatsPointer makeBeatGrid(
            mixxx::audio::SampleRate sampleRate,
            double dBpm,
            double dFirstBeatSample);

    static QString getPreferredVersion(bool bEnableFixedTempoCorrection);

    static QString getPreferredSubVersion(
            bool bEnableFixedTempoCorrection,
            bool bEnableOffsetCorrection,
            int iMinBpm,
            int iMaxBpm,
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool bEnableFixedTempoCorrection,
            bool bEnableOffsetCorrection,
            mixxx::audio::SampleRate iSampleRate,
            SINT totalSamples,
            int iMinBpm,
            int iMaxBpm);
};
