#pragma once

#include <QVector>
#include <memory>

#include "audio/frame.h"
#include "audio/streaminfo.h"
#include "track/beats.h"

namespace mixxx {

/// Importer class for Beats that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class BeatsImporter {
  public:
    virtual ~BeatsImporter() = default;

    virtual bool isEmpty() const = 0;

    /// Determines the timing offset and returns a Beats object.
    virtual BeatsPointer importBeatsAndApplyTimingOffset(
            const QString& filePath,
            const QString& fileType,
            const audio::StreamInfo& streamInfo) = 0;
};

typedef std::shared_ptr<BeatsImporter> BeatsImporterPointer;

} // namespace mixxx
