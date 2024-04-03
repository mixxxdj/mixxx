#pragma once

#include "track/steminfo.h"

namespace mixxx {

/// Importer class for StemInfo objects that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class StemInfoImporter {
  public:
    static QList<StemInfo> importStemInfos(
            const QString& filePath);

    static bool isStemFile(
            const QString& aFileName);
};

} // namespace mixxx
