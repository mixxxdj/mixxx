#include "util/time.h"

// static
LLTIMER Time::s_timer;
// static
bool Time::s_testMode = false;
// static
qint64 Time::s_testElapsed_nsecs = 0;
