#pragma once

#include <QByteArray>
#include <QLoggingCategory>
#include <QtGlobal>

namespace mixxx::qml {

inline bool qmlRenderDiagnosticsEnabled() {
    static const bool enabled = [] {
        const QByteArray value = qgetenv("MIXXX_QML_RENDER_DIAGNOSTICS");
        return !value.isEmpty() && value != QByteArrayLiteral("0") &&
                value.compare(QByteArrayLiteral("false"), Qt::CaseInsensitive) != 0;
    }();
    return enabled;
}

inline bool qmlRenderForceFullSurface() {
    static const bool enabled = [] {
        const QByteArray value = qgetenv("MIXXX_QML_RENDER_FORCE_FULL_SURFACE");
        return !value.isEmpty() && value != QByteArrayLiteral("0") &&
                value.compare(QByteArrayLiteral("false"), Qt::CaseInsensitive) != 0;
    }();
    return enabled;
}

inline const QLoggingCategory& qmlRenderDiagnosticsCategory() {
    static const QLoggingCategory category("qml.render.diagnostics");
    return category;
}

} // namespace mixxx::qml
