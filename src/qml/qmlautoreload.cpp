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
    m_pathTriggeringReload.insert(url.toLocalFile());
    return url;
}

void QmlAutoReload::slotFileChanged(const QString& changedFile) {
    qDebug() << "File" << changedFile << "used in QML interface has been changed.";
    if (m_pathTriggeringReload.remove(changedFile)) {
        m_fileWatcher.removePath(changedFile);
        emit triggered();
    }
}

void QmlAutoReload::clear() {
    const auto files = m_fileWatcher.files();
    if (!files.isEmpty()) {
        m_fileWatcher.removePaths(files);
    }
}

} // namespace qml
} // namespace mixxx
