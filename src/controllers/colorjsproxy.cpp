#include "controllers/colorjsproxy.h"

#include "preferences/colorpalettesettings.h"
#include "util/color/rgbcolor.h"

ColorJSProxy::ColorJSProxy(QScriptEngine* pScriptEngine,
        ColorPaletteSettings colorPaletteSettings)
        : m_pScriptEngine(pScriptEngine),
          m_JsHotcueColorPalette(
                  makeHotcueColorPalette(pScriptEngine, colorPaletteSettings)),
          m_colorPaletteSettings(colorPaletteSettings) {
}

ColorJSProxy::~ColorJSProxy() = default;

Q_INVOKABLE QScriptValue ColorJSProxy::hotcueColorPalette() {
    return m_JsHotcueColorPalette;
}

QScriptValue ColorJSProxy::colorFromHexCode(uint colorCode) {
    QRgb rgb = QRgb(colorCode);
    QScriptValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", qRed(rgb));
    jsColor.setProperty("green", qGreen(rgb));
    jsColor.setProperty("blue", qBlue(rgb));
    jsColor.setProperty("alpha", qAlpha(rgb));
    return jsColor;
}

QScriptValue ColorJSProxy::makeHotcueColorPalette(QScriptEngine* pScriptEngine,
        ColorPaletteSettings colorPaletteSettings) {
    // TODO: make sure we get notified when the palette changes
    QList<mixxx::RgbColor> colorList = colorPaletteSettings.getHotcueColorPalette().m_colorList;
    int numColors = colorList.length();
    QScriptValue jsColorList = pScriptEngine->newArray(numColors);
    for (int i = 0; i < numColors; ++i) {
        mixxx::RgbColor color = colorList.at(i);
        jsColorList.setProperty(i, colorFromHexCode(color));
    }
    return jsColorList;
}
