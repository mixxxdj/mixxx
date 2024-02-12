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
    if (!url.isLocalFile() || !QFileInfo(url.toLocalFile()).isFile()) {
        return url;
    }
    m_fileWatcher.addPath(url.toLocalFile());
    return url;
}

void QmlAutoReload::slotFileChanged(const QString& changedFile) {
    qDebug() << "File" << changedFile << "used in QML interface has been changed.";
    // This is to prevent double-reload when a file is updated twice
    // in a row as part of the normal saving process. See note in
    // QFileSystemWatcher::fileChanged documentation.
    if (m_fileWatcher.removePath(changedFile)) {
        emit triggered();
    }
}

void QmlAutoReload::clear() {
    m_fileWatcher.removePaths(m_fileWatcher.files());
}

} // namespace qml
} // namespace mixxx
