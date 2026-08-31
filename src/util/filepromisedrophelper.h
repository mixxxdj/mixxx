#pragma once

#include <QtGlobal>

#ifdef Q_OS_MACOS

#include <QList>
#include <QPoint>
#include <QUrl>
#include <functional>

namespace mixxx {
namespace mac {

class FilePromiseDropHelper {
  public:
    using DropCallback = std::function<void(const QList<QUrl>& urls, const QPoint& globalPos)>;

    /// Registers Cocoa drag hooks on QNSView to intercept NSFilePromiseReceiver drops.
    static void initialize(DropCallback callback);

    /// Cleans up stale promised file downloads in the cache directory from previous sessions.
    static void purgeStaleCache();
};

} // namespace mac
} // namespace mixxx

#endif // Q_OS_MACOS
