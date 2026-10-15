#pragma once

#include <QString>

namespace mixxx {
namespace skin {

/// Reads the application wide default stylesheet (skins/default.qss) from the
/// given resource path.
///
/// It styles the native widgets that both the widget based main window and the
/// QML application host, for example the library, the menus and the dialogs.
/// Returns an empty string when the file is missing.
QString loadDefaultStyleSheet(const QString& resourcePath);

} // namespace skin
} // namespace mixxx
