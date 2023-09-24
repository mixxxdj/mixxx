#include "qml/qmlconfigproxy.h"

#include "moc_qmlconfigproxy.cpp"
#include "preferences/colorpalettesettings.h"

namespace {
QVariantList paletteToQColorList(const ColorPalette& palette) {
    QVariantList colors;
    for (mixxx::RgbColor rgbColor : palette) {
        colors.append(mixxx::RgbColor::toQVariantColor(rgbColor));
    }
    return colors;
}
} // namespace

namespace mixxx {
namespace qml {

QmlConfigProxy::QmlConfigProxy(
        UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent), m_pConfig(pConfig) {
}

QVariantList QmlConfigProxy::getHotcueColorPalette() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getHotcueColorPalette());
}

QVariantList QmlConfigProxy::getTrackColorPalette() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getTrackColorPalette());
}

// static
QmlConfigProxy* QmlConfigProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pQmlEngine);

    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    DEBUG_ASSERT(s_pInstance);

    // The engine has to have the same thread affinity as the singleton.
    DEBUG_ASSERT(pJsEngine->thread() == s_pInstance->thread());

    // There can only be one engine accessing the singleton.
    if (s_pJsEngine) {
        DEBUG_ASSERT(pJsEngine == s_pJsEngine);
    } else {
        s_pJsEngine = pJsEngine;
    }

    // Explicitly specify C++ ownership so that the engine doesn't delete
    // the instance.
    QJSEngine::setObjectOwnership(s_pInstance, QJSEngine::CppOwnership);
    return s_pInstance;
}

} // namespace qml
} // namespace mixxx
