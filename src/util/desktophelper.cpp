
#include "desktophelper.h"

#include <QtDebug>
#include <QString>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QProcess>

namespace {
    QString sSelectInFileBrowserCommand;

    QString getOpenInFileBrowserCommand() {
        return "nemo --select %1";
        return ""; // no special command use QDesktopServices";
    }

    QString removeChildDir(const QString& path) {
        int index = path.lastIndexOf(QChar('/'));
        return path.left(index);
    }
} // anonymous namespace


namespace mixxx {

void DesktopHelper::openInFileBrowser(const QStringList& paths) {
    if (sSelectInFileBrowserCommand.isNull()) {
        sSelectInFileBrowserCommand = getOpenInFileBrowserCommand();
    }
    QSet<QString> openedDirs;
    QString dirPath;
    for (const auto& path: paths) {
        dirPath = removeChildDir(path);
        if (!sSelectInFileBrowserCommand.isEmpty() &&
                QFile::exists(path)) {
            if (!openedDirs.contains(dirPath)) {
                openedDirs.insert(dirPath);
                // special command, that supports also select the requested file
                QString command = sSelectInFileBrowserCommand.arg(
                        QUrl::fromLocalFile(path).toString());
                qDebug() << "starting:" << command;
                qDebug() << "parent:" << dirPath;
                QProcess::startDetached(command);
            }
        } else {
            QDir dir = dirPath;
            while (!dir.exists() && dirPath.size()) {
                dirPath = removeChildDir(dirPath);
                dir = dirPath;
            }

            // This function does not work for a non-existent directory!
            // so it is essential that in the worst case it try opening
            // a valid directory, in this case, 'QDir::home()'.
            // Otherwise nothing would happen...
            if (!dir.exists()) {
                // it ensures a valid dir for any OS (Windows)
                dir = QDir::home();
            }
            // not open the same dir twice
            dirPath = dir.absolutePath();
            qDebug() << "opening:" << dirPath;
            if (!openedDirs.contains(dirPath)) {
                openedDirs.insert(dirPath);
                QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
            }
        }
    }
}

} // namespace mixxx

