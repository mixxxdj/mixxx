#pragma once

#include "track/steminfo.h"

namespace mixxx {

/// Importer class for StemInfo objects that contains metadata about the stems of a track.
class StemInfoImporter {
  public:
    static QList<StemInfo> importStemInfos(
            const QString& filePath);

    static bool isStemFile(
            const QString& aFileName);
};

} // namespace mixxx
