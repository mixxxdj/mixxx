#include "qml/qmlautoreload.h"

#include <QDebug>
#include <QFileInfo>
#include <QUrl>

#include "moc_qmlautoreload.cpp"

namespace mixxx {

namespace qml {

QmlAutoReload::QmlAutoReload() {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &QmlAutoReload::slotFileChanged);
}

QUrl QmlAutoReload::intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
    const auto path = url.toLocalFile();

    if (!url.isLocalFile() || !QFileInfo(path).isFile()) {
        return url;
    }

    m_fileWatcher.addPath(path);
    return url;
}

void QmlAutoReload::slotFileChanged(const QString& changedFile) {
    qDebug() << "File" << changedFile << "used in QML interface has been changed.";

    // Unfortunately we cannot be sure that `QFileSystemWatcher::removePath()`
    // actually returns true, so we need to emit the signal anyway.
    m_fileWatcher.removePath(changedFile);
    emit triggered();
}

void QmlAutoReload::clear() {
    const auto files = m_fileWatcher.files();
    if (!files.isEmpty()) {
        m_fileWatcher.removePaths(files);
    }
}

} // namespace qml
} // namespace mixxx
