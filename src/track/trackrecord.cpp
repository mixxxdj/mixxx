#include "track/trackrecord.h"


namespace mixxx {

TrackRecord::TrackRecord(TrackId id)
        : m_id(std::move(id)),
          m_cuePoint(0.0),
          m_rating(0),
          m_metadataParsed(false),
          m_bpmLocked(false) {
}


} //namespace mixxx
