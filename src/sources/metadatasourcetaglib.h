#pragma once

#include "sources/metadatasource.h"

namespace mixxx {

// Universal default implementation of IMetadataSource using TagLib.
class MetadataSourceTagLib : public MetadataSource {
  public:
    MetadataSourceTagLib(
            const QString& fileName,
            const QString& fileType)
            : m_fileName(fileName),
              m_fileType(taglib::stringToEnumFileType(fileType)) {
    }

    std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt,
            bool resetMissingTagMetadata) const override;

    std::pair<ExportResult, QDateTime> exportTrackMetadata(
            const TrackMetadata& trackMetadata) const override;

  private:
    std::pair<ImportResult, QDateTime> afterImport(ImportResult importResult) const;
    std::pair<ExportResult, QDateTime> afterExport(ExportResult exportResult) const;

    QString m_fileName;
    taglib::FileType m_fileType;
};

} // namespace mixxx
