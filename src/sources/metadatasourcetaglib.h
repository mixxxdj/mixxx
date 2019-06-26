#pragma once

#include "sources/metadatasource.h"

#include "track/trackmetadatataglib.h"

namespace mixxx {

// Universal default implementation of IMetadataSource using TagLib.
class MetadataSourceTagLib : public MetadataSource {
  public:
    explicit MetadataSourceTagLib(
            QString fileName)
            : m_fileName(fileName),
              m_fileType(taglib::getFileTypeFromFileName(fileName)) {
    }
    MetadataSourceTagLib(
            QString fileName,
            taglib::FileType fileType)
            : m_fileName(fileName),
              m_fileType(fileType) {
    }

    std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    std::pair<ExportResult, QDateTime> exportTrackMetadata(
            const TrackMetadata& trackMetadata) const override;

  private:
    std::pair<ImportResult, QDateTime> afterImport(ImportResult importResult) const;
    std::pair<ExportResult, QDateTime> afterExport(ExportResult exportResult) const;

    QString m_fileName;
    taglib::FileType m_fileType;
};

} //namespace mixxx
