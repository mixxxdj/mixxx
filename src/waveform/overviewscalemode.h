#pragma once

// required for Qt-Macros
#include <qobjectdefs.h>

namespace mixxx {
Q_NAMESPACE

enum class OverviewScaleMode {
    FileLevel,
    Normalize,
    AllGainReplayGain
};
Q_ENUM_NS(OverviewScaleMode);

} // namespace mixxx
