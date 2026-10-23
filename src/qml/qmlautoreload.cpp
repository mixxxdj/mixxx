#include "qml/qmlautoreload.h"

#include <QDebug>
#include <QFileInfo>
#include <QUrl>

#include "moc_qmlautoreload.cpp"
#include "util/autofilereloader.h"

namespace mixxx {

namespace qml {

QmlAutoReload::QmlAutoReload()
        : m_autoReloader(RuntimeLoggingCategory(QStringLiteral("qml_auto_reload"))) {
    connect(&m_autoReloader,
            &AutoFileReloader::fileChanged,
            this,
            [this](const QString& changedFile) {
                emit triggered();
            });
}

QUrl QmlAutoReload::intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType) {
    const auto generation = m_generation.load();
    if (!url.isLocalFile()) {
        return url;
    }
    QString filePath = url.toLocalFile();
    if (!QFileInfo(filePath).isFile()) {
        return url;
    }
    QMetaObject::invokeMethod(this, [this, filePath, generation]() {
                if (generation == m_generation.load()) {
                    m_autoReloader.addPath(filePath);
                } }, Qt::AutoConnection);
    return url;
}

} // namespace qml
} // namespace mixxx
