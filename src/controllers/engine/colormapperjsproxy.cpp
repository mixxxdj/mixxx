#include "colormapperjsproxy.h"

ColorMapperJSProxy::ColorMapperJSProxy(QJSEngine* pScriptEngine, QMap<QRgb, QVariant> availableColors)
        : m_pScriptEngine(pScriptEngine),
          m_colorMapper(new ColorMapper(availableColors)) {
}

QJSValue ColorMapperJSProxy::getNearestColor(uint colorCode) {
    QRgb result = m_colorMapper->getNearestColor(static_cast<QRgb>(colorCode));
    QJSValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", qRed(result));
    jsColor.setProperty("green", qGreen(result));
    jsColor.setProperty("blue", qBlue(result));
    return jsColor;
}

QJSValue ColorMapperJSProxy::getValueForNearestColor(uint colorCode) {
    return m_pScriptEngine->toScriptValue(
            m_colorMapper->getValueForNearestColor(static_cast<QRgb>(colorCode)));
}
