#include "controllers/colorjsproxy.h"
#include "preferences/hotcuecolorpalettesettings.h"

ColorJSProxy::ColorJSProxy(
        QScriptEngine* pScriptEngine, UserSettingsPointer pConfig)
        : m_pScriptEngine(pScriptEngine),
          m_hotcueColorPalette(makeHotcueColorPalette(pScriptEngine, pConfig)),
          m_pConfig(pConfig) {
}

ColorJSProxy::~ColorJSProxy() = default;

Q_INVOKABLE QScriptValue ColorJSProxy::hotcueColorPalette() {
    return m_hotcueColorPalette;
}

QScriptValue ColorJSProxy::colorFromHexCode(uint colorCode) {
    QColor color = QColor::fromRgba(colorCode);
    QScriptValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", color.red());
    jsColor.setProperty("green", color.green());
    jsColor.setProperty("blue", color.blue());
    jsColor.setProperty("alpha", color.alpha());
    return jsColor;
}

QScriptValue ColorJSProxy::makeHotcueColorPalette(
        QScriptEngine* pScriptEngine, UserSettingsPointer pConfig) {
    // TODO: make sure we get notified when the palette changes
    HotcueColorPaletteSettings colorPaletteSettings(pConfig);
    QList<QColor> colorList =
            colorPaletteSettings.getHotcueColorPalette().m_colorList;
    int numColors = colorList.length();
    QScriptValue jsColorList = pScriptEngine->newArray(numColors);
    for (int i = 0; i < numColors; ++i) {
        QColor color = colorList.at(i);
        jsColorList.setProperty(i, colorFromHexCode(color.rgba()));
    }
    return jsColorList;
}
