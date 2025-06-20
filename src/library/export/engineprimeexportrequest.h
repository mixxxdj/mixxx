#pragma once

#include <QDir>
#include <QSet>
#include <djinterop/djinterop.hpp>

#include "library/trackset/crate/crateid.h"

namespace mixxx {

/// A request to export the Mixxx library to an external Engine Prime database.
struct EnginePrimeExportRequest {
    /// Directory path, ending in "Engine Library", where database files will
    /// be written.
    QDir engineLibraryDbDir;

    /// Directory in which to write the exported music files.
    QDir musicFilesDir;

    /// Version of Engine Prime database schema to use when exporting.
    djinterop::engine::engine_schema exportSchemaVersion;

    /// Set of crates to export.
    ///
    /// An empty set of crates AND playlists to export implies that the whole
    /// music library is to be exported.
    QSet<CrateId> crateIdsToExport;

    /// Set of playlists to export,
    ///
    /// An empty set of crates AND playlists to export implies that the whole
    /// music library is to be exported.
    QSet<int> playlistIdsToExport;
};

} // namespace mixxx
