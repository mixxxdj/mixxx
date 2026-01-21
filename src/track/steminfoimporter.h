#pragma once

#include <QByteArray>
#include <QMimeType>

#include "track/steminfo.h"

namespace mixxx {

/// Importer class for StemInfo objects that contains metadata about the stems of a track.
class StemInfoImporter {
  public:
    static StemInfo importStemInfos(
            const QByteArray& manifest);

    static QByteArray exportStemInfos(
            const StemInfo& manifest);

    // checkFileContent allows to check the file audio stream topology to see if
    // it contains valid streams requirement. This is useful to create or
    // support steam track which don't the tag atom yet.
    static bool maybeStemFile(const QString& aFileName,
            QMimeType mimeType = QMimeType(),
            bool checkFileContent = false);

    static bool hasStemAtom(
            const QString& aFileName);
};

} // namespace mixxx
