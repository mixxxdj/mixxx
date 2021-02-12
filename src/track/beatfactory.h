#pragma once

#include <QHash>

#include "audio/types.h"
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

    static QString getPreferredVersion(bool fixedTempo);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(const Track& track,
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool fixedTempo,
            const mixxx::audio::SampleRate& sampleRate);

  private:
    static void deleteBeats(mixxx::Beats* pBeats);
};
