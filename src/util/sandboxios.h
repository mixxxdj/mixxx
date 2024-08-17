#pragma once

#include <QString>
#include <QtGlobal>

namespace mixxx {

#ifdef Q_OS_IOS

/// Updates a path into an iOS sandbox with the current sandbox prefix.
/// This is needed since iOS rotates the UUID in such paths whenever
/// the app is reinstalled or updated. These paths are of the form
///
///     /private/var/mobile/Containers/Data/Application/<uuid>
///
QString updateIOSSandboxPath(const QString& path);

#endif

}; // namespace mixxx
