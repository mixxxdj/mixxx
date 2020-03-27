#include "controllers/engine/colorjsproxy.h"
#include "controllers/engine/colormapperjsproxy.h"
#include "controllers/engine/controllerengine.h"

ColorJSProxy::ColorJSProxy(ControllerEngine* pControllerEngine)
        : m_pControllerEngine(pControllerEngine) {}

ColorJSProxy::~ColorJSProxy() = default;


QJSValue ColorJSProxy::colorMapper(const QMap<QRgb, QVariant>& colorMap) {
    QMap<QRgb, QVariant> availableColors;

    QMapIterator<QRgb, QVariant> it(colorMap);
    while (it.hasNext()) {
        it.next();
        QColor color(it.key());
        if (color.isValid()) {
            availableColors.insert(color.rgb(), it.value());
        } else {
            m_pControllerEngine->throwJSError(
                    QStringLiteral("Invalid color name passed to ColorMapper: ") + QString::number(it.key()));
            continue;
        }
    }

    if (availableColors.isEmpty()) {
        m_pControllerEngine->throwJSError(
                QStringLiteral("Failed to create ColorMapper object:' available colors mustn't be empty!"));
        return QJSValue(QJSValue::UndefinedValue);
    }

    QObject* colorMapper = new ColorMapperJSProxy(m_pControllerEngine->m_pScriptEngine, availableColors);
    return m_pControllerEngine->m_pScriptEngine->newQObject(colorMapper);
}