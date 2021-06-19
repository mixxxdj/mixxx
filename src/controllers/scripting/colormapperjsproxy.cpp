#include "controllers/scripting/colormapperjsproxy.h"

#include "moc_colormapperjsproxy.cpp"

ColorMapperJSProxy::ColorMapperJSProxy(const QVariantMap& availableColors)
        : m_pColorMapper(nullptr) {
    QMap<QRgb, QVariant> qrgbMap;
    QVariantMap::const_iterator it;
    for (it = availableColors.constBegin(); it != availableColors.constEnd(); ++it) {
        bool isInt = false;
        QRgb color = it.key().toInt(&isInt);
        if (isInt) {
            qrgbMap.insert(color, it.value());
        }
    }
    m_pColorMapper = std::make_unique<ColorMapper>(qrgbMap);
}

ColorMapperJSProxy::ColorMapperJSProxy()
        : m_pColorMapper(nullptr) {
    qWarning() << "ColorMapper constructor called without a map of colors";
}

QVariantMap ColorMapperJSProxy::getNearestColor(uint colorCode) {
    // Do not VERIFY_OR_DEBUG_ASSERT here because the problem is the mapping
    // script calling the default constructor, not an error in the C++ code.
    if (m_pColorMapper == nullptr) {
        return QVariantMap();
    }

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
    // Do not VERIFY_OR_DEBUG_ASSERT here because the problem is the mapping
    // script calling the default constructor, not an error in the C++ code.
    if (m_pColorMapper == nullptr) {
        return QVariant();
    }

    return m_pColorMapper->getValueForNearestColor(static_cast<QRgb>(colorCode));
}
