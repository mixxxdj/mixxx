#pragma once

#include <QString>

#include "track/track_decl.h"
#include "track/trackid.h"

/// A small, thread-safe snapshot of the track data that Auto DJ 2.0 plus Video Mixing needs
/// for mixing decisions. It is copied out of `Track` on the GUI thread so
/// that the scorer and sequencer can run on a worker thread without ever
/// touching `Track` objects or the database.
struct TrackFeatures {
    TrackId id;
    double bpm = 0.0;          ///< 0 = unknown
    int camelotNumber = 0;     ///< 1..12, 0 = unknown
    bool camelotMinor = false; ///< true = "A" (minor), false = "B" (major)
    double energy = 0.0;       ///< 1..10, 0 = unknown
    /// true = the DJ rated this energy by hand; false = measured by
    /// AnalyzerEnergy (less trustworthy, so it counts for less).
    bool energyIsManual = false;
    double loudnessLufs = 0.0; ///< integrated loudness, 0 = unknown
    double durationSec = 0.0;
    bool isStem = false;
    QString displayName; ///< "Artist - Title" (or file name), for messages
    QString artist;      ///< as tagged (may be empty)
    QString title;       ///< as tagged (may be empty)
    QString genre;       ///< as tagged (may be empty)
    /// A music video (by file type), for video sets: bridges between videos
    /// should be videos too.
    bool isVideo = false;
    /// The beat grid check found that the grid drifts off the music: Auto DJ
    /// cannot beatmatch this track (it switches quickly instead).
    bool gridUnsteady = false;

    /// True for the file types that hold video (mp4, mov, mkv, ...).
    static bool isVideoFile(const QString& location) {
        const QString suffix = location.section(QChar('.'), -1).toLower();
        return suffix == QStringLiteral("mp4") || suffix == QStringLiteral("m4v") ||
                suffix == QStringLiteral("mov") || suffix == QStringLiteral("mkv") ||
                suffix == QStringLiteral("webm") || suffix == QStringLiteral("avi") ||
                suffix == QStringLiteral("wmv") || suffix == QStringLiteral("mpg") ||
                suffix == QStringLiteral("mpeg");
    }

    bool hasBpm() const {
        return bpm > 0.0;
    }
    bool hasKey() const {
        return camelotNumber >= 1 && camelotNumber <= 12;
    }
    bool hasEnergy() const {
        return energy > 0.0;
    }

    /// Camelot notation, e.g. "8A". Returns "?" when the key is unknown.
    QString camelotText() const {
        if (!hasKey()) {
            return QStringLiteral("?");
        }
        return QString::number(camelotNumber) +
                (camelotMinor ? QChar('A') : QChar('B'));
    }

    /// Reads BPM, key, ReplayGain and stem info from a loaded Track.
    /// Must be called on a thread that may access the Track.
    static TrackFeatures fromTrack(const TrackPointer& pTrack);

    /// Converts an OpenKey number (1..12) to a Camelot number (1..12).
    /// Returns 0 for an invalid input. OpenKey 1 (C major / A minor) = Camelot 8.
    static int camelotFromOpenKey(int openKeyNumber);
};
