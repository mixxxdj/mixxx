#include "track/beat.h"

const int kFrameSize = 2;

Beat::Beat()
        : m_iIndex(0) {
}

Beat::Beat(const mixxx::track::io::Beat& protoBeat)
        : m_iIndex(0) {
    m_beat.set_frame_position(protoBeat.frame_position());
    m_beat.set_type(protoBeat.type());
    m_beat.set_enabled(protoBeat.enabled());
    m_beat.set_source(protoBeat.source());
}

int Beat::getSamplePosition() const {
    return getFramePosition() * kFrameSize;
}

void Beat::setSamplePosition(int samplePosition) {
    setFramePosition(samplePosition / kFrameSize);
}

bool operator<(const Beat& beat1, const Beat& beat2) {
    return beat1.getFramePosition() < beat2.getFramePosition();
}
