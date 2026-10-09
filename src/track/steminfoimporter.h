#pragma once

#include <QMimeType>

#include "track/steminfo.h"

namespace mixxx {

/// Importer class for StemInfo objects that contains metadata about the stems of a track.
class StemInfoImporter {
  public:
    static QList<StemInfo> importStemInfos(
            const QString& filePath);

    static bool maybeStemFile(const QString& filePath);
    static bool hasStemAtom(const QString& filePath);
    static bool hasStemFileExtension(const QString& filePath);
    static bool isStemMimeType(const QMimeType& mimeType);
};

} // namespace mixxx
