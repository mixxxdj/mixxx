#pragma once

#include <QLoggingCategory>

#include "util/cmdlineargs.h"

namespace mixxx::qml {

inline bool qmlRenderDiagnosticsEnabled() {
    return CmdlineArgs::Instance().getQmlRenderDiagnostics();
}

inline bool qmlRenderForceFullSurface() {
    return CmdlineArgs::Instance().getQmlRenderForceFullSurface();
}

inline const QLoggingCategory& qmlRenderDiagnosticsCategory() {
    static const QLoggingCategory category("qml.render.diagnostics");
    return category;
}

} // namespace mixxx::qml
