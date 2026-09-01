#include "qml/qmlautoreload.h"

#include <QDebug>
#include <QFileInfo>
#include <QUrl>

#include "moc_qmlautoreload.cpp"
#include "util/autofilereloader.h"
#include "util/qmldiagnostics.h"

namespace mixxx {

namespace qml {

QmlAutoReload::QmlAutoReload()
        : m_autoReloader(RuntimeLoggingCategory(QStringLiteral("qml_auto_reload"))) {
    connect(&m_autoReloader,
            &AutoFileReloader::fileChanged,
            this,
            [this](const QString& changedFile) {
                if (qmlRenderDiagnosticsEnabled()) {
                    qCDebug(qmlRenderDiagnosticsCategory())
                            << "QmlAutoReload triggered" << changedFile;
                }
                emit triggered();
            });
}

QUrl QmlAutoReload::intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType) {
    if (!url.isLocalFile()) {
        return url;
    }
    QString filePath = url.toLocalFile();
    if (!QFileInfo(filePath).isFile()) {
        return url;
    }
    if (qmlRenderDiagnosticsEnabled()) {
        qCDebug(qmlRenderDiagnosticsCategory())
                << "QmlAutoReload watching" << filePath;
    }
    m_autoReloader.addPath(filePath);
    return url;
}

} // namespace qml
} // namespace mixxx
