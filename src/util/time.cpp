#include "util/time.h"

namespace mixxx {

// static
Time::LLTIMER Time::s_timer;

// static
bool Time::s_testMode = false;

// static
Time::time_point Time::s_testElapsed = Time::time_point::min();

} // namespace mixxx
