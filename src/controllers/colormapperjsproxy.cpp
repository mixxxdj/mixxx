#include "controllers/colormapperjsproxy.h"

#include <QScriptValueIterator>

#include "moc_colormapperjsproxy.cpp"

ColorMapperJSProxy::ColorMapperJSProxy(QScriptEngine* pScriptEngine,
        const QMap<QRgb, QVariant>& availableColors)
        : m_pScriptEngine(pScriptEngine),
          m_colorMapper(new ColorMapper(availableColors)) {
}

QScriptValue ColorMapperJSProxy::getNearestColor(uint colorCode) {
    QRgb result = m_colorMapper->getNearestColor(static_cast<QRgb>(colorCode));
    QScriptValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", qRed(result));
    jsColor.setProperty("green", qGreen(result));
    jsColor.setProperty("blue", qBlue(result));
    return jsColor;
}

QScriptValue ColorMapperJSProxy::getValueForNearestColor(uint colorCode) {
    return m_pScriptEngine->toScriptValue(
            m_colorMapper->getValueForNearestColor(static_cast<QRgb>(colorCode)));
}

QScriptValue ColorMapperJSProxyConstructor(QScriptContext* pScriptContext, QScriptEngine* pScriptEngine) {
    QMap<QRgb, QVariant> availableColors;
    if (pScriptContext->argumentCount() != 1) {
        pScriptContext->throwError(
                QStringLiteral("Failed to create ColorMapper object: constructor takes exactly one argument!"));
        return pScriptEngine->undefinedValue();
    }
    QScriptValue argument = pScriptContext->argument(0);
    if (!argument.isValid() || !argument.isObject()) {
        pScriptContext->throwError(
                QStringLiteral("Failed to create ColorMapper object: argument needs to be an object!"));
        return pScriptEngine->undefinedValue();
    }

    QScriptValueIterator it(argument);
    while (it.hasNext()) {
        it.next();
        bool isInt = false;
        QColor color(it.name().toInt(&isInt));
        if (isInt && color.isValid()) {
            availableColors.insert(color.rgb(), it.value().toVariant());
        } else {
            pScriptContext->throwError(
                    QStringLiteral("Invalid color name passed to ColorMapper: ") + it.name());
            continue;
        }
    }

    if (availableColors.isEmpty()) {
        pScriptContext->throwError(
                QStringLiteral("Failed to create ColorMapper object: available colors mustn't be empty!"));
        return pScriptEngine->undefinedValue();
    }

    QObject* colorMapper = new ColorMapperJSProxy(pScriptEngine, availableColors);
    return pScriptEngine->newQObject(colorMapper, QScriptEngine::ScriptOwnership);
}
