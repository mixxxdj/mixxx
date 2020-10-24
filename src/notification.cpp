#include "notification.h"

namespace mixxx {

Notification::Notification(const QString& text, NotificationFlags flags)
        : m_flags(flags), m_text(text){};

} // namespace mixxx
