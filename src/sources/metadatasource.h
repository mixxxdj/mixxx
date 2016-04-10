#ifndef MIXXX_METADATASOURCE_H
#define MIXXX_METADATASOURCE_H

#include <QImage>

#include "track/trackmetadata.h"
#include "util/result.h"

namespace Mixxx {

// Interface for parsing track metadata and cover art.
class MetadataSource {
public:
    // Read both track metadata and cover art at once, because this
    // is should be the most common use case. Both parameters are
    // output parameters and might be NULL if their result is not
    // needed.
    virtual Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const = 0;

    // Update track metadata of the source.
    virtual Result writeTrackMetadata(
            const TrackMetadata& trackMetadata) const = 0;

protected:
    virtual ~MetadataSource() {}
};

} //namespace Mixxx

#endif // MIXXX_METADATASOURCE_H
