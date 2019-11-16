#include <QScriptValueIterator>

#include "controllers/colormapperjsproxy.h"

ColorMapperJSProxy::ColorMapperJSProxy(QScriptEngine* pScriptEngine, QMap<QRgb, QVariant> availableColors)
    : m_pScriptEngine(pScriptEngine) {
        m_colorMapper = new ColorMapper(availableColors);
}

QScriptValue ColorMapperJSProxy::getNearestColor(uint colorCode) {
    QPair<QRgb, QVariant> result = m_colorMapper->getNearestColor(static_cast<QRgb>(colorCode));
    return m_pScriptEngine->toScriptValue(result.second);
}

QScriptValue ColorMapperJSProxyConstructor(QScriptContext* pScriptContext, QScriptEngine* pScriptEngine) {
    QMap<QRgb, QVariant> availableColors;
    DEBUG_ASSERT(pScriptContext->argumentCount() == 1);
    QScriptValueIterator it(pScriptContext->argument(0));
    while (it.hasNext()) {
        it.next();
        DEBUG_ASSERT(!it.value().isObject());
        QColor color(it.name());
        VERIFY_OR_DEBUG_ASSERT(color.isValid()) {
            qWarning() << "Received invalid color name from controller script:" << it.name();
            continue;
        }
        availableColors.insert(color.rgb(), it.value().toVariant());
    }

    if (availableColors.isEmpty()) {
        qWarning() << "Failed to create ColorMapper object: available colors mustn't be empty!";
        return pScriptEngine->undefinedValue();
    }

    QObject* colorMapper = new ColorMapperJSProxy(pScriptEngine, availableColors);
    return pScriptEngine->newQObject(colorMapper, QScriptEngine::ScriptOwnership);
}
