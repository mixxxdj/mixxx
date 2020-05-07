#ifdef __DJINTEROP__
#pragma once

#include <QDir>

#include "library/export/exportrequest.h"

namespace mixxx {

/// A request to export the Mixxx library to an external Engine Library database.
struct EngineLibraryExportRequest : public ExportRequest {
    /// Directory in which to write the Engine Library database files.
    QDir engineLibraryDbDir;

    /// Directory in which to write the exported music files.
    QDir musicFilesDir;
};

} // namespace mixxx
#endif
