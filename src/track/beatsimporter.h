#pragma once

#include <QVector>
#include <memory>

#include "audio/streaminfo.h"

namespace mixxx {

/// Importer class for Beats that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class BeatsImporter {
  public:
    BeatsImporter() = default;
    virtual ~BeatsImporter() = default;

    virtual bool isEmpty() const {
        return true;
    };

    /// Determines the timing offset and returns a Vector of frame positions
    /// to use as input for the BeatMap constructor
    virtual QVector<double> importBeatsAndApplyTimingOffset(
            const QString& filePath,
            const audio::StreamInfo& streamInfo) {
        Q_UNUSED(filePath);
        Q_UNUSED(streamInfo);
        return {};
    };
};

typedef std::shared_ptr<BeatsImporter> BeatsImporterPointer;

} // namespace mixxx
