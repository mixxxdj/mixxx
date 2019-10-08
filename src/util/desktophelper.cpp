
#include "util/desktophelper.h"

#include <QtDebug>
#include <QString>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QProcess>

#ifdef Q_OS_LINUX
#include <QtDBus/QtDBus>
#include <QFileInfo>
#endif

namespace {

const QString kSelectInFreedesktop = "fd";
const QString kSelectInXfce = "xf";
QString sSelectInFileBrowserCommand;

QString getSelectInFileBrowserCommand() {
#if defined(Q_OS_MAC)
    return "open -R";
#elif defined(Q_OS_WIN)
    return "explorer.exe /select,";
#elif defined(Q_OS_LINUX)
    QProcess proc;
    QString output;
    // if xdg-mime is not installed, it uses the else branch below.
    proc.start("xdg-mime",
            QStringList() << "query" << "default" << "inode/directory");
    proc.waitForFinished();
    output = proc.readLine().simplified();
    if (output == "kfmclient_dir.desktop") {
        return "konqueror --select";
    } else if (output == "Thunar.desktop" ||
            output == "Thunar-folder-handler.desktop") {
        return kSelectInXfce; // Thunar has no --select option
    } else {
        // Known file manager with org.freedesktop.FileManager1 interface
        // * Nautilus
        // * Nemo
        // * Deepin File Manager
        // * Swordfish
        // * Caja
        // No solution for
        // * pcmanfm
        return kSelectInFreedesktop; // no special command, try Freedesktop and fallback to QDesktopServices";
    }
#else
    return ""; // no special command use QDesktopServices";
#endif
}

QString removeChildDir(const QString& path) {
    int index = path.lastIndexOf(QChar('/'));
    return path.left(index);
}

#ifdef Q_OS_LINUX
bool selectInFreedesktop(const QString& path) {
    const QUrl fileurl = QUrl::fromLocalFile(path);
    const QStringList args(fileurl.toString());
    QDBusMessage msg = QDBusMessage::createMethodCall(
            "org.freedesktop.FileManager1",
            "/org/freedesktop/FileManager1",
            "org.freedesktop.FileManager1",
            "ShowItems");
    msg << args << "";
    const QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    const bool success = (response.type() != QDBusMessage::MessageType::ErrorMessage);
    return success;
}

bool selectInXfce(const QString& path) {
    const QString folder = QFileInfo(path).dir().absolutePath();
    const QString filename = QFileInfo(path).fileName();

    QDBusMessage msg = QDBusMessage::createMethodCall(
            "org.xfce.FileManager",
            "/org/xfce/FileManager",
            "org.xfce.FileManager",
            "DisplayFolderAndSelect");
    msg << folder << filename << "" << "";
    qDebug() << "Calling:" << msg;
    const QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    const bool success = (response.type() != QDBusMessage::MessageType::ErrorMessage);
    return success;
}
#endif

void selectViaCommand(const QString& path) {
    QStringList arguments = sSelectInFileBrowserCommand.split(" ");
    // No escaping required because QProcess bypasses the shell
    arguments.append(QDir::toNativeSeparators(path));
    QString program = arguments.takeFirst();
    qDebug() << "Calling:" << program << arguments;
    QProcess::startDetached(program, arguments);
}

} // anonymous namespace


namespace mixxx {

void DesktopHelper::openInFileBrowser(const QStringList& paths) {
    if (sSelectInFileBrowserCommand.isNull()) {
        sSelectInFileBrowserCommand = getSelectInFileBrowserCommand();
    }
    QSet<QString> openedDirs; // We only select the first selected file in a folder
    QString dirPath;
    for (const auto& path: paths) {
        QFileInfo fileInfo(path);
        dirPath = fileInfo.absolutePath();

        if (openedDirs.contains(dirPath)) {
            // Folder already opened and we cannot select two files in the same folder
            continue;
        }

        if (fileInfo.exists()) {
            // Tryto select the file
#ifdef Q_OS_LINUX
            if (sSelectInFileBrowserCommand == kSelectInFreedesktop) {
                if (selectInFreedesktop(path)) {
                    openedDirs.insert(dirPath);
                    continue;
                } else {
                    qDebug() << "Select File via Freedesktop DBus interface failed";
                }
            } else if (sSelectInFileBrowserCommand == kSelectInXfce) {
                if (selectInXfce(path)) {
                    openedDirs.insert(dirPath);
                    continue;
                } else {
                    qDebug() << "Select file via Xfce DBus interface failed";
                }
            } else
#endif
            if (!sSelectInFileBrowserCommand.isEmpty()) {
                // special command, that supports selecting the requested file
                selectViaCommand(path);
                openedDirs.insert(dirPath);
                continue;
            }
        }

        // We cannot select, just open the parent folder
        QDir dir = dirPath;
        while (!dir.exists() && dirPath.size()) {
            // Note: dir.cdUp() does not work for not existing dirs
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
            QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
            openedDirs.insert(dirPath);
        }
    }
}

} // namespace mixxx

