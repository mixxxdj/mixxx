#include "sources/metadatasource.h"

#include "track/trackmetadatataglib.h"


namespace mixxx {

Result TracklibMetadataSource::parseTrackMetadataAndCoverArt(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt) const {
    return taglib::readTrackMetadataAndCoverArtFromFile(pTrackMetadata, pCoverArt, m_fileName);
}

Result TracklibMetadataSource::writeTrackMetadata(
        const TrackMetadata& trackMetadata) const {
    return taglib::writeTrackMetadataIntoFile(trackMetadata, m_fileName);
}

} // namespace mixxx
