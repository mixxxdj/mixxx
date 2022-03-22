#include "sources/metadatasource.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("MetadataSource");

} // anonymous namespace

// static
QDateTime MetadataSource::getFileSynchronizedAt(const QFile& file) {
    // Directly obtain the modification time from the file to prevent
    // using stale data due to internal caching in QFileInfo!
    const QDateTime lastModifiedUtc = file.fileTime(QFileDevice::FileModificationTime).toUTC();
    // Ignore bogus values like 1970-01-01T00:00:00.000 UTC
    // that are obviously incorrect and don't provide any
    // information.
    if (lastModifiedUtc.isValid() &&
            // Only defined if valid
            lastModifiedUtc.toMSecsSinceEpoch() == 0) {
        return {};
    }
    return lastModifiedUtc;
}

} // namespace mixxx
