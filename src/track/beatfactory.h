#pragma once

#include <QHash>

#include "track/beats.h"

class Track;

class BeatFactory {
  public:
    static mixxx::BeatsPointer loadBeatsFromByteArray(
            SINT sampleRat,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);
    static mixxx::BeatsPointer makeBeatGrid(
            SINT sampleRat,
            double dBpm,
            double dFirstBeatSample);

    static QString getPreferredVersion(const bool bEnableFixedTempoCorrection);

    static QString getPreferredSubVersion(
            const bool bEnableFixedTempoCorrection,
            const bool bEnableOffsetCorrection,
            const int iMinBpm,
            const int iMaxBpm,
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            const bool bEnableFixedTempoCorrection,
            const bool bEnableOffsetCorrection,
            const int iSampleRate,
            const int iTotalSamples,
            const int iMinBpm,
            const int iMaxBpm);
};
