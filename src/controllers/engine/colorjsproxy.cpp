#include "controllers/engine/colorjsproxy.h"

ColorJSProxy::ColorJSProxy(QJSEngine* pScriptEngine)
        : m_pScriptEngine(pScriptEngine),
          m_predefinedColorsList(makePredefinedColorsList(pScriptEngine)){};

ColorJSProxy::~ColorJSProxy(){};

QJSValue ColorJSProxy::predefinedColorFromId(int iId) {
    PredefinedColorPointer color(Color::kPredefinedColorsSet.predefinedColorFromId(iId));
    return jsColorFrom(color);
};

Q_INVOKABLE QJSValue ColorJSProxy::predefinedColorsList() {
    return m_predefinedColorsList;
}

QJSValue ColorJSProxy::jsColorFrom(PredefinedColorPointer predefinedColor) {
    QJSValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", predefinedColor->m_defaultRgba.red());
    jsColor.setProperty("green", predefinedColor->m_defaultRgba.green());
    jsColor.setProperty("blue", predefinedColor->m_defaultRgba.blue());
    jsColor.setProperty("alpha", predefinedColor->m_defaultRgba.alpha());
    jsColor.setProperty("id", predefinedColor->m_iId);
    return jsColor;
}

QJSValue ColorJSProxy::makePredefinedColorsList(QJSEngine* pScriptEngine) {
    int numColors = Color::kPredefinedColorsSet.allColors.length();
    QJSValue colorList = pScriptEngine->newArray(numColors);
    for (int i = 0; i < numColors; ++i) {
        PredefinedColorPointer color = Color::kPredefinedColorsSet.allColors.at(i);
        colorList.setProperty(i, jsColorFrom(color));
    }
    return colorList;
}
