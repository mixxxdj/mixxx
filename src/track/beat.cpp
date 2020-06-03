#include "track/beat.h"

const int kFrameSize = 2;

void Beat::init(const mixxx::track::io::Beat& protoBeat) {
    set_frame_position(protoBeat.frame_position());
    set_type(protoBeat.type());
    set_enabled(protoBeat.enabled());
    set_source(protoBeat.source());
}

float Beat::sample_position() {
    return frame_position() * kFrameSize;
}
