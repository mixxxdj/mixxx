#pragma once

#include <QString>

namespace mixxx {

/// Writes a native Mixxx STEM file (.stem.mp4): an MP4 container holding 5
/// AAC stereo streams — stream 0 is the main mix, streams 1..4 are the
/// individual stems (vocals, drums, bass, other) — plus the `moov/udta/stem`
/// atom containing the STEM manifest JSON, exactly as expected by
/// SoundSourceSTEM and StemInfoImporter.
///
/// The output file is therefore loadable by Mixxx's native stem system
/// (stem_count == 4, native stem volume/mute controls, native stem UI).
class StemMp4Writer {
public:
    struct Config {
        QString outputPath;
        int sampleRate = 44100;
        /// Interleaved stereo float buffers (L,R,L,R,...), one per stem, in
        /// order: vocals, drums, bass, other. May be nullptr if a stem should
        /// be silent (it will be encoded as silence).
        const float* stems[4] = {};
        /// Number of frames (stereo sample pairs) per stem.
        qint64 numFrames = 0;
        QString stemNames[4] = {"Vocals", "Drums", "Bass", "Other"};
        QString stemColors[4] = {"#FD4A4A", "#00E8E8", "#FFFF00", "#AD65FF"};
    };

    /// Writes the stem file and returns true on success. The file is written
    /// atomically (via a temp file + rename) so a failed write never leaves a
    /// half-written .stem.mp4 behind.
    static bool write(const Config& config);
};

} // namespace mixxx
