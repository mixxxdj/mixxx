#pragma once

#include <QDateTime>
#include <QImage>

#include <utility>

#include "track/trackmetadata.h"
#include "util/memory.h"

namespace mixxx {

// API and abstract base class for parsing track metadata and
// cover art.
//
// The time stamp returned from the source when importing/exporting
// metadata reflects the current version of the metadata at the source
// and can be used for synchronization purposes.
class MetadataSource {
  public:
    virtual ~MetadataSource() {
    }

    enum class ImportResult {
        Succeeded,
        Failed,
        Unavailable,
    };

    // Read both track metadata and cover art at once, because this
    // is the most common use case. Both pointers are output parameters
    // and might be passed a nullptr if their result is not needed.
    // If no metadata is available for a track then the source should
    // return Unavailable as this default implementation does.
    virtual std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* /*pTrackMetadata*/,
            QImage* /*pCoverImage*/) const {
        return std::make_pair(ImportResult::Unavailable, QDateTime());
    }

    enum class ExportResult {
        Succeeded,
        Failed,
        Unsupported,
    };

    // Update track metadata of the source.
    // Sources that are read-only and don't support updating of metadata
    // should return Unsupported as this default implementation does.
    virtual std::pair<ExportResult, QDateTime> exportTrackMetadata(
            const TrackMetadata& /*trackMetadata*/) const {
        return std::make_pair(ExportResult::Unsupported, QDateTime());
    }
};

typedef std::shared_ptr<MetadataSource> MetadataSourcePointer;

} //namespace mixxx
