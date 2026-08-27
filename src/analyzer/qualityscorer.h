#pragma once

#include <QString>
#include <QtGlobal>

// Lightweight quality fields read from the library + track_locations tables.
// Used in place of TrackPointer so this code is safe to call from background
// threads without touching the GlobalTrackCache.
struct TrackQualityFields {
    QString filetype;
    int sampleRate{0};
    qint64 fileSize{0};
};

class QualityScorer {
  public:
    static double calculateScore(const TrackQualityFields& fields);

  private:
    static double baseScoreForFormat(const QString& ext);
};
