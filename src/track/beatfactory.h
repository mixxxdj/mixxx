#pragma once

#include <QHash>

#include "audio/types.h"
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

    static QString getPreferredVersion(bool fixedTempo);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<double>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool fixedTempo,
            const mixxx::audio::SampleRate& sampleRate);
};
