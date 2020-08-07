#pragma once

#include "track/trackref.h"


class RelocatedTrack final {
  public:
    RelocatedTrack() = default;
    RelocatedTrack(RelocatedTrack&&) = default;
    RelocatedTrack(const RelocatedTrack&) = default;
    RelocatedTrack& operator=(RelocatedTrack&&) = default;
    RelocatedTrack& operator=(const RelocatedTrack&) = default;

    RelocatedTrack(
            TrackRef missingTrackRef,
            TrackRef addedTrackRef)
            : m_missingTrackRef(std::move(missingTrackRef)),
              m_addedTrackRef((std::move(addedTrackRef))) {
        DEBUG_ASSERT(m_missingTrackRef.isValid());
        DEBUG_ASSERT(m_missingTrackRef.hasId());
        DEBUG_ASSERT(m_missingTrackRef.hasLocation());
        // NOTE: m_addedTrackRef might not be valid!
        // It might not have a valid TrackId if the relocated track has
        // not been added as a new track to the internal collection before.
        // It may also not have a valid canonical location, e.g. when
        // relinking directories and the track is missing at both the old
        // and the new location.
        DEBUG_ASSERT(m_addedTrackRef.hasLocation());
        DEBUG_ASSERT(updatedTrackRef().isValid());
        DEBUG_ASSERT(updatedTrackRef().hasId());
        DEBUG_ASSERT(updatedTrackRef().hasLocation());
    }

    // The new, updated TrackRef of the relocated track after merging
    // the missing with the newly added track.
    TrackRef updatedTrackRef() const {
        return TrackRef(
                m_addedTrackRef,
                m_missingTrackRef.getId());
    }

    // The newly added track has been removed after merging (optional).
    const TrackId& deletedTrackId() const {
        return m_addedTrackRef.getId();
    }

    // The old track location was missing and has been removed
    // after merging.
    const QString& deletedTrackLocation() const {
        return m_missingTrackRef.getLocation();
    }

  private:
    TrackRef m_missingTrackRef;
    TrackRef m_addedTrackRef;
};

Q_DECLARE_METATYPE(RelocatedTrack);
