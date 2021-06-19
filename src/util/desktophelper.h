#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QStringList);

namespace mixxx {

class DesktopHelper {
public:
   static void openInFileBrowser(const QStringList& paths);
};

} // namespace mixxx