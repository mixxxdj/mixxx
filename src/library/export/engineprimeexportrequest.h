#pragma once

#include <QDir>
#include <djinterop/djinterop.hpp>

#include "library/export/exportrequest.h"

namespace mixxx {

/// A request to export the Mixxx library to an external Engine Prime database.
struct EnginePrimeExportRequest : public ExportRequest {
    /// Directory path, ending in "Engine Library", where database files will
    /// be written.
    QDir engineLibraryDbDir;

    /// Directory in which to write the exported music files.
    QDir musicFilesDir;

    /// Version of Engine Prime database to use when exporting.
    djinterop::semantic_version exportVersion;
};

} // namespace mixxx
