#ifndef MIXXX_METADATASOURCE_H
#define MIXXX_METADATASOURCE_H

#include <QImage>

#include "track/trackmetadata.h"
#include "util/result.h"

namespace mixxx {

// Interface for parsing track metadata and cover art.
class /*interface*/ IMetadataSource {
  public:
    virtual ~IMetadataSource() {}

    // Read both track metadata and cover art at once, because this
    // is usually the most common use case. Both parameters are
    // output parameters and might be passed as nullptr if their
    // result is not needed.
    virtual Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const = 0;

    // Update track metadata of the source.
    virtual Result writeTrackMetadata(
            const TrackMetadata& trackMetadata) const = 0;
};

// Universal default implementation of IMetadataSource
class TracklibMetadataSource: public virtual /*interface*/ IMetadataSource {
  public:
    explicit TracklibMetadataSource(QString fileName)
        : m_fileName(fileName) {
    }

    Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    Result writeTrackMetadata(
            const TrackMetadata& trackMetadata) const override;

  private:
    QString m_fileName;
};

} //namespace mixxx

#endif // MIXXX_METADATASOURCE_H
