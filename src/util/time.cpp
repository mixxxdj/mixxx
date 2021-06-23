#include "util/time.h"

namespace mixxx {

// static
LLTIMER Time::s_timer;

// static
bool Time::s_testMode = false;

// static
Duration Time::s_testElapsed = Duration::fromNanos(0);

} // namespace mixxx
