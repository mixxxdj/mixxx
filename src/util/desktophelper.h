#pragma once

#include <QUrl>
#include <QtGlobal>

namespace mixxx {

class DesktopHelper {
public:
   static void openInFileBrowser(const QStringList& paths);

   static bool openUrl(const QUrl& url);
};

} // namespace mixxx
