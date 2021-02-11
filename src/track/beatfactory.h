#pragma once

#include <QHash>

#include "track/beats.h"

class Track;

class BeatFactory {
  public:
    static mixxx::BeatsPointer loadBeatsFromByteArray(const Track& track,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);
    static mixxx::BeatsPointer makeBeatGrid(const Track& track,
            double dBpm,
            double dFirstBeatSample);

    static QString getPreferredVersion(const bool bEnableFixedTempoCorrection);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(const Track& track,
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            const bool bEnableFixedTempoCorrection,
            const int iSampleRate);

  private:
    static void deleteBeats(mixxx::Beats* pBeats);
};
