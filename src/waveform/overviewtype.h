#pragma once

// required for Qt-Macros
#include <qobjectdefs.h>

namespace mixxx {
Q_NAMESPACE

enum class OverviewType {
    Filtered,
    HSV,
    RGB,
    RGB3Band,
};
Q_ENUM_NS(OverviewType);

} // namespace mixxx
