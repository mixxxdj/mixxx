#pragma once

#include "sources/metadatasource.h"

#include "track/trackmetadatataglib.h"


namespace mixxx {

// Universal default implementation of IMetadataSource using TagLib.
class MetadataSourceTagLib: public MetadataSource {
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

    ImportResult importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    ExportResult exportTrackMetadata(
            const TrackMetadata& trackMetadata) const override;

  private:
    QString m_fileName;
    taglib::FileType m_fileType;
};

} //namespace mixxx
