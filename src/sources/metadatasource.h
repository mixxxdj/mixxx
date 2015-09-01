#ifndef MIXXX_METADATASOURCE_H
#define MIXXX_METADATASOURCE_H

#include "metadata/trackmetadata.h"
#include "util/defs.h" // Result

#include <QImage>

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

protected:
    virtual ~MetadataSource() {}
};

} //namespace Mixxx

#endif // MIXXX_METADATASOURCE_H
