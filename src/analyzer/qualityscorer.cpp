#include "analyzer/qualityscorer.h"

#include <QHash>
#include <QtDebug>

namespace {
const bool sDebugQualityScorer = true;
} // namespace

// static
double QualityScorer::baseScoreForFormat(const QString& ext) {
    // Lossless formats cluster at 100,
    // lossy formats fall off based on typical perceptual
    // quality at common encode settings --
    // not a measured property of any individual file.
    static const QHash<QString, double> kScores = {
            {"flac", 100.0},
            {"aiff", 100.0},
            {"aif", 100.0},
            {"alac", 100.0},
            {"wav", 90.0},
            {"m4a", 85.0},
            {"aac", 65.0},
            {"mp3", 60.0},
            {"ogg", 55.0},
            {"opus", 55.0},
            {"wma", 50.0},
    };
    return kScores.value(ext.toLower(), 50.0);
}

// static
double QualityScorer::calculateScore(const TrackQualityFields& fields) {
    double score = baseScoreForFormat(fields.filetype);

    // Sample-rate bonus -- proposal's modifier, +5 above 44.1 kHz.
    if (fields.sampleRate > 44100) {
        score += 5.0;
    }

    // Note: the proposal's prose also mentions "+3 for bit depths above
    // 16-bit", but bit depth isn't a queryable column on library or
    // track_locations -- it only exists as JSON inside track_analysis
    // (type='audio_quality', written by the future fake-lossless detector).
    // Wiring that up means joining track_analysis and parsing JSON here,
    // which is a bigger change than this PR. Deferred to whichever PR adds
    // fake-lossless detection, since it will need to parse that JSON anyway.

    // File-size bonus as a rough bitrate proxy, capped at +5. This is the
    // weakest part of the formula -- file size conflates bitrate with
    // track length, so a long lossy file can outscore a short lossless
    // one on this term alone. Flagged for revisiting, not fixed in this PR.
    score += qMin(5.0, static_cast<double>(fields.fileSize) / (10 * 1024 * 1024));

    if (sDebugQualityScorer) {
        qDebug() << "QualityScorer -> [calculateScore] -> result"
                 << "filetype:" << fields.filetype
                 << "sampleRate:" << fields.sampleRate
                 << "fileSize:" << fields.fileSize
                 << "score:" << score;
    }
    return score;
}
