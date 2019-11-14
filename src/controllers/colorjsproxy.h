#ifndef COLORJSPROXY_H
#define COLORJSPROXY_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptValue>

#include "preferences/hotcuecolorpalettesettings.h"
#include "util/color/color.h"

class ColorJSProxy final : public QObject {
    Q_OBJECT
  public:
    ColorJSProxy(QScriptEngine* pScriptEngine,
            HotcueColorPaletteSettings colorPaletteSettings);

    ~ColorJSProxy() override;

    Q_INVOKABLE QScriptValue hotcueColorPalette();
    // Return a JS object with the red, green, blue and alpha components
    // of a color. The parameter is the hexadecimal representation of the color
    // i.e. 0xAARRGGBB
    Q_INVOKABLE QScriptValue colorFromHexCode(uint colorCode);
    Q_INVOKABLE QScriptValue nearestColorMidiCode(uint colorCode, QVariantMap availableColorCodes);

  private:
    QScriptValue makeHotcueColorPalette(QScriptEngine* pScriptEngine,
            HotcueColorPaletteSettings colorPaletteSettings);
    QScriptEngine* m_pScriptEngine;
    QScriptValue m_JsHotcueColorPalette;
    HotcueColorPaletteSettings m_colorPaletteSettings;
};

#endif /* COLORJSPROXY_H */
