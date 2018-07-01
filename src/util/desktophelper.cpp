
#include "desktophelper.h"

#include <QtDebug>
#include <QString>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QProcess>

namespace {
    QString sSelectInFileBrowserCommand;

    QString getSelectInFileBrowserCommand() {
#if defined(Q_OS_MAC)
        return "open -R \"%1\"";
#elif defined(Q_OS_WIN)
        return "explorer.exe /select,\"%1\"";
#elif defined(Q_OS_LINUX)
        QProcess proc;
        QString output;
        proc.start("xdg-mime",
                QStringList() << "query" << "default" << "inode/directory");
        proc.waitForFinished();
        output = proc.readLine().simplified();
        if (output == "dolphin.desktop" ||
                output == "org.kde.dolphin.desktop") {
            return "dolphin --select \"%1\"";
        } else if (output == "nautilus.desktop" ||
                output == "org.gnome.Nautilus.desktop" ||
                output == "nautilus-folder-handler.desktop") {
            return "nautilus --no-desktop --select \"%1\"";
        } else if (output == "caja-folder-handler.desktop") {
            return ""; // caja has no --select option
        } else if (output == "pcmanfm.desktop") {
            return ""; // pcmanfm has no --select option
        } else if (output == "nemo.desktop") {
            return "nautilus --no-desktop --select \"%1\"";
        } else if (output == "kfmclient_dir.desktop") {
            return "konqueror --select \"%1\"";
        } else if (output == "Thunar.desktop") {
            return ""; // Thunar has no --select option
        } else {
            qDebug() << "xdg-mime" << output << "unknown, can't select track in file browser";
            return ""; // no special command use QDesktopServices";
        }
#else
        return ""; // no special command use QDesktopServices";
#endif
    }

    QString removeChildDir(const QString& path) {
        int index = path.lastIndexOf(QChar('/'));
        return path.left(index);
    }
} // anonymous namespace


namespace mixxx {

void DesktopHelper::openInFileBrowser(const QStringList& paths) {
    if (sSelectInFileBrowserCommand.isNull()) {
        sSelectInFileBrowserCommand = getSelectInFileBrowserCommand();
    }
    QSet<QString> openedDirs;
    QString dirPath;
    for (const auto& path: paths) {
        dirPath = removeChildDir(path);
        // First try to select the file in file browser
        if (!sSelectInFileBrowserCommand.isEmpty() &&
                QFile::exists(path)) {
            if (!openedDirs.contains(dirPath)) {
                openedDirs.insert(dirPath);
                // special command, that supports also select the requested file
                QString command = sSelectInFileBrowserCommand.arg(
                        QDir::toNativeSeparators(path));
                qDebug() << "starting:" << command;
                qDebug() << "parent:" << dirPath;
                QProcess::startDetached(command);
            }
        } else {
            // We cannot select, just open the parent folder
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

