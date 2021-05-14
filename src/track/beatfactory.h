#pragma once

#include <QHash>

#include "audio/types.h"
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

    static QString getPreferredVersion(bool fixedTempo);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool fixedTempo,
            mixxx::audio::SampleRate sampleRate);
};
