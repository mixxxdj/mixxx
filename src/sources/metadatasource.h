#ifndef METADATASOURCE_H
#define METADATASOURCE_H

#include "metadata/trackmetadata.h"

#include <QImage>

namespace Mixxx {

// Interface for metadata
class MetadataSource {
public:
    // Only metadata that is quickly readable should be read.
    // The implementation is free to set inaccurate estimated
    // values here.
    virtual Result parseTrackMetadata(TrackMetadata* pMetadata) const = 0;

    // Returns the first cover art image embedded within the
    // file (if any).
    virtual QImage parseCoverArt() const = 0;

protected:
    virtual ~MetadataSource() {}
};

} //namespace Mixxx

#endif // METADATASOURCE_H
