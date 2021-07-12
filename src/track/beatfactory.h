#pragma once

#include <QHash>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/beats.h"
#include "track/bpm.h"

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
            mixxx::Bpm bpm,
            mixxx::audio::FramePos firstBeatFramePos);

    static QString getPreferredVersion(bool fixedTempo);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<mixxx::audio::FramePos>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool fixedTempo,
            mixxx::audio::SampleRate sampleRate);
};
