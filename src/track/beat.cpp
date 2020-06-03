#include "track/beat.h"

void Beat::init(const mixxx::track::io::Beat& protoBeat) {
    set_frame_position(protoBeat.frame_position());
    set_type(protoBeat.type());
    set_enabled(protoBeat.enabled());
    set_source(protoBeat.source());
}
