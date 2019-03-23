#ifndef MIXXX_DESKTOPHELPER_H
#define MIXXX_DESKTOPHELPER_H

class QStringList;

namespace mixxx {

class DesktopHelper {
public:
   static void openInFileBrowser(const QStringList& paths);
};

}

#endif // MIXXX_DESKTOPHELPER_H
