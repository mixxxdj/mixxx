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
    // Return a JS object with the red, green and blue components
    // of a color. The parameter is the hexadecimal representation of the color
    // i.e. 0xRRGGBB
    Q_INVOKABLE QScriptValue colorFromRgb(uint rgb);

  private:
    QScriptValue makeHotcueColorPalette(QScriptEngine* pScriptEngine,
            HotcueColorPaletteSettings colorPaletteSettings);
    QScriptEngine* m_pScriptEngine;
    QScriptValue m_JsHotcueColorPalette;
    HotcueColorPaletteSettings m_colorPaletteSettings;
};

#endif /* COLORJSPROXY_H */
