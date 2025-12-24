#pragma once

#include "util/types.h"

namespace mixxx {

// In the worst case up to 29 MP3 frames need to be prefetched
// for accurate seeking:
// http://www.mars.org/mailman/public/mad-dev/2002-May/000634.html
// Used by both SoundSourceMp3 and SoundSourceCoreAudio.
constexpr SINT kMp3SeekFramePrefetchCount = 29;

} // namespace mixxx
