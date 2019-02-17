#include "controllers/colorjsproxy.h"

ColorJSProxy::ColorJSProxy(QScriptEngine* pScriptEngine)
    : m_pScriptEngine(pScriptEngine),
      m_predefinedColorsList(makePredefinedColorsList(pScriptEngine)){};

ColorJSProxy::~ColorJSProxy() {};

QScriptValue ColorJSProxy::predefinedColorFromId(int iId) {
    PredefinedColorPointer color(Color::predefinedColorSet.predefinedColorFromId(iId));
    return jsColorFrom(color);
};

Q_INVOKABLE QScriptValue ColorJSProxy::predefinedColorsList() {
    return m_predefinedColorsList;
}

QScriptValue ColorJSProxy::jsColorFrom(PredefinedColorPointer predefinedColor) {
    QScriptValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", predefinedColor->m_defaultRgba.red());
    jsColor.setProperty("green", predefinedColor->m_defaultRgba.green());
    jsColor.setProperty("blue", predefinedColor->m_defaultRgba.blue());
    jsColor.setProperty("alpha", predefinedColor->m_defaultRgba.alpha());
    jsColor.setProperty("id", predefinedColor->m_iId);
    return jsColor;
}

QScriptValue ColorJSProxy::makePredefinedColorsList(QScriptEngine* pScriptEngine) {
    int numColors = Color::predefinedColorSet.allColors.length();
    QScriptValue colorList = pScriptEngine->newArray(numColors);
    for (int i = 0; i < numColors; ++i) {
        PredefinedColorPointer color = Color::predefinedColorSet.allColors.at(i);
        colorList.setProperty(i, jsColorFrom(color));
    }
    return colorList;
}
