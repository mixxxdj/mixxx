#pragma once

// required for Qt-Macros
#include <qobjectdefs.h>

namespace mixxx {

namespace control {

Q_NAMESPACE

enum class ButtonMode {
    Push,
    Toggle,
    PowerWindow,
    LongPressLatching,
    Trigger
};

Q_ENUM_NS(ButtonMode);

} // namespace control
} // namespace mixxx
