
#include "desktophelper.h"

#include <QtDebug>
#include <QString>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

namespace {
    QString sOpenInFileBrowserCommand;
    QString getOpenInFileBrowserCommand() {
        return ""; // no special command use QDesktopServices";
    }
} // anonymous namespace


namespace mixxx {

void DesktopHelper::openInFileBrowser(const QStringList& paths) {
    if (sOpenInFileBrowserCommand.isNull()) {
        sOpenInFileBrowserCommand = getOpenInFileBrowserCommand();
    }
    QSet<QString> openedDirs;
    QDir dir;
    for (const auto& path: paths) {
        QStringList splittedPath = path.split("/");
        do {
            dir = splittedPath.join("/");
            splittedPath.removeLast();
        } while (!dir.exists() && splittedPath.size());

        // This function does not work for a non-existent directory!
        // so it is essential that in the worst case it try opening
        // a valid directory, in this case, 'QDir::home()'.
        // Otherwise nothing would happen...
        if (!dir.exists()) {
            // it ensures a valid dir for any OS (Windows)
            dir = QDir::home();
        }
        // not open the same dir twice
        QString dirPath = dir.absolutePath();
        if (!openedDirs.contains(dirPath)) {
            openedDirs.insert(dirPath);
            if (sOpenInFileBrowserCommand.isEmpty()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
            } else {
                // special command, that supports also select the requested file
            }
        }
    }
}

} // namespace mixxx

