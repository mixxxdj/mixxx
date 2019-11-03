#ifndef COLORJSPROXY_H
#define COLORJSPROXY_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptValue>

#include "preferences/usersettings.h"
#include "util/color/color.h"

class ColorJSProxy final : public QObject {
    Q_OBJECT
  public:
    ColorJSProxy(QScriptEngine* pScriptEngine, UserSettingsPointer pConfig);

    ~ColorJSProxy() override;

    Q_INVOKABLE QScriptValue hotcueColorPalette();
    // Return a JS object with the red, green, blue and alpha components
    // of a color. The parameter is the hexadecimal representation of the color
    // i.e. 0xAARRGGBB
    Q_INVOKABLE QScriptValue colorFromHexCode(uint colorCode);

  private:
    QScriptValue makeHotcueColorPalette(
            QScriptEngine* pScriptEngine, UserSettingsPointer pConfig);
    QScriptEngine* m_pScriptEngine;
    QScriptValue m_hotcueColorPalette;
    UserSettingsPointer m_pConfig;
};

#endif /* COLORJSPROXY_H */
