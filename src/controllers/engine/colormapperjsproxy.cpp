#include "controllers/engine/colormapperjsproxy.h"

ColorMapperJSProxy::ColorMapperJSProxy(QVariantMap availableColors)
        : m_pColorMapper(nullptr) {
    QMap<QRgb, QVariant> qrgbMap;
    QVariantMap::const_iterator it;
    for (it = availableColors.constBegin(); it != availableColors.constEnd(); ++it) {
        QString colorString = it.key();
        bool isInt = false;
        if (colorString.at(0) == "#") {
            colorString = colorString.right(colorString.size() - 1);
            int fromHex = colorString.toInt(&isInt, 16);
            if (isInt) {
                qrgbMap.insert(fromHex, it.value());
            }
            continue;
        }

        QRgb color = colorString.toInt(&isInt);
        if (isInt) {
            qrgbMap.insert(color, it.value());
        }
    }
    m_pColorMapper = std::make_unique<ColorMapper>(qrgbMap);
}

QVariantMap ColorMapperJSProxy::getNearestColor(uint colorCode) {
    QRgb result = m_pColorMapper->getNearestColor(static_cast<QRgb>(colorCode));
    // Constructing a QJSValue without using QJSEngine::newObject creates
    // an undefined JS object. Using QJSValue::setProperty on that default
    // constructed object does not work. However, QJSEngine converts the
    // QVariantMap returned from this function to a JS object.
    QVariantMap jsColor;
    jsColor.insert("red", qRed(result));
    jsColor.insert("green", qGreen(result));
    jsColor.insert("blue", qBlue(result));
    return jsColor;
}

QVariant ColorMapperJSProxy::getValueForNearestColor(uint colorCode) {
    return m_pColorMapper->getValueForNearestColor(static_cast<QRgb>(colorCode));
}
