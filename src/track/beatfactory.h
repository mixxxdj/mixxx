#pragma once

#include <QHash>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/beats.h"
#include "track/bpm.h"

class Track;

class BeatFactory {
  public:
    static QString getPreferredVersion(bool fixedTempo);

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(
            const QVector<mixxx::audio::FramePos>& beats,
            const QHash<QString, QString>& extraVersionInfo,
            bool fixedTempo,
            mixxx::audio::SampleRate sampleRate);
};
