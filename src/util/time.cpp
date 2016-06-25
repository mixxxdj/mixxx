#include "util/time.h"

// static
LLTIMER Time::s_timer;

// static
bool Time::s_testMode = false;

// static
mixxx::Duration Time::s_testElapsed = mixxx::Duration::fromNanos(0);
