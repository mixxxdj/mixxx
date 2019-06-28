#include "controllers/colorjsproxy.h"
#include "controllers/controllerengine.h"

ColorJSProxy::ColorJSProxy(QScriptEngine* pScriptEngine)
        : m_pScriptEngine(pScriptEngine),
          m_predefinedColorsList(makePredefinedColorsList()) {
}

ColorJSProxy::~ColorJSProxy() {
}

QScriptValue ColorJSProxy::predefinedColorFromId(int iId) {
    return m_predefinedColorsList.property(iId);
}

Q_INVOKABLE QScriptValue ColorJSProxy::predefinedColorsList() {
    return m_predefinedColorsList;
}

void ColorJSProxy::setDefaultColor(const QColor& defaultColor) {
    m_predefinedColorsList = makePredefinedColorsList(defaultColor);
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

QScriptValue ColorJSProxy::makePredefinedColorsList(QColor defaultColorRgba) {
    int numColors = Color::kPredefinedColorsSet.allColors.length();
    QScriptValue colorList = m_pScriptEngine->newArray(numColors);
    PredefinedColorPointer predefinedDefaultColor = Color::kPredefinedColorsSet.noColor;
    PredefinedColorPointer skinTunedDefaultColor = std::make_shared<PredefinedColor>(
            defaultColorRgba,
            predefinedDefaultColor->m_sName,
            predefinedDefaultColor->m_sDisplayName,
            predefinedDefaultColor->m_iId);
    colorList.setProperty(0, jsColorFrom(skinTunedDefaultColor));
    for (int i = 1; i < numColors; ++i) {
        PredefinedColorPointer color = Color::kPredefinedColorsSet.allColors.at(i);
        colorList.setProperty(i, jsColorFrom(color));
    }
    return colorList;
}
