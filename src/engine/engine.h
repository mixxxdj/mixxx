#pragma once

#include "util/audiosignal.h"


namespace mixxx {
    // TODO(XXX): When we move from stereo to multi-channel this needs updating.
    static constexpr mixxx::AudioSignal::ChannelCount kEngineChannelCount(2);
}
