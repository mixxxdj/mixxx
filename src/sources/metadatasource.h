#pragma once

#include <QDateTime>
#include <QFile>
#include <QImage>
#include <memory>
#include <optional>
#include <utility>

#include "track/trackmetadata.h"

namespace mixxx {

// API and abstract base class for parsing track metadata and
// cover art.
//
// The time stamp returned from the source when importing/exporting
// metadata reflects the current version of the metadata at the source
// and can be used for synchronization purposes.
class MetadataSource {
  public:
    virtual ~MetadataSource() = default;

    /// Get the synchronization time of the given file.
    ///
    /// Includes special case handling to detect bogus time stamps.
    static QDateTime getFileSynchronizedAt(const QFile& file);

    enum class ImportResult {
        Succeeded,
        Failed,
        Unavailable,
    };

    /// Read both track metadata and cover art at once, because this
    /// is the most common use case. Both pointers are output parameters
    /// and might be passed a nullptr if their result is not needed.
    /// If no metadata is available for a track then the source should
    /// return Unavailable as this default implementation does.
    /// The flag resetMissingTagMetadata controls if existing metadata
    /// should be discarded if the corresponding file tags are missing.
    virtual std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverImage,
            bool resetMissingTagMetadata) const {
        Q_UNUSED(pTrackMetadata)
        Q_UNUSED(pCoverImage)
        Q_UNUSED(resetMissingTagMetadata)
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

    /// Import rating from file tags using FMPS_Rating standard.
    /// Returns the rating (0-5) if found, nullopt otherwise.
    /// Default implementation returns nullopt (no rating support).
    virtual std::optional<int> importRating() const {
        return std::nullopt;
    }

    /// Export rating to file tags using FMPS_Rating standard.
    /// Returns true if export succeeded, false otherwise.
    /// Default implementation returns false (no rating support).
    virtual bool exportRating(int /*rating*/) const {
        return false;
    }
};

typedef std::shared_ptr<MetadataSource> MetadataSourcePointer;

} // namespace mixxx
