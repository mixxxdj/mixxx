#include <cstdint>
#include <cmath>
#include "controllers/colorjsproxy.h"
#include "preferences/hotcuecolorpalettesettings.h"

namespace {
    double colorDistance(QColor a, QColor b)
    {
        long mean_red = ((long)a.red() + (long)b.red()) / 2;
        long delta_red = (long)a.red() - (long)b.red();
        long delta_green = (long)a.green() - (long)b.green();
        long delta_blue = (long)a.blue() - (long)b.blue();
        return sqrt(
            (((512 + mean_red) * delta_red * delta_red) >> 8) +
            (4 * delta_green * delta_green) +
            (((767 - mean_red) * delta_blue * delta_blue) >> 8)
        );
    }
}


ColorJSProxy::ColorJSProxy(QScriptEngine* pScriptEngine,
        HotcueColorPaletteSettings colorPaletteSettings)
        : m_pScriptEngine(pScriptEngine),
          m_JsHotcueColorPalette(
                  makeHotcueColorPalette(pScriptEngine, colorPaletteSettings)),
          m_colorPaletteSettings(colorPaletteSettings) {
}

ColorJSProxy::~ColorJSProxy() = default;

Q_INVOKABLE QScriptValue ColorJSProxy::hotcueColorPalette() {
    return m_JsHotcueColorPalette;
}

QScriptValue ColorJSProxy::colorFromHexCode(uint colorCode) {
    QColor color = QColor::fromRgba(colorCode);
    QScriptValue jsColor = m_pScriptEngine->newObject();
    jsColor.setProperty("red", color.red());
    jsColor.setProperty("green", color.green());
    jsColor.setProperty("blue", color.blue());
    jsColor.setProperty("alpha", color.alpha());
    return jsColor;
}

QScriptValue ColorJSProxy::nearestColorMidiCode(uint colorCode, QVariantMap availableColors) {
    QColor desiredColor = QColor::fromRgba(colorCode);
    uint nearestColorValue = 0;
    double nearestColorDistance = qInf();
    QMapIterator<QString, QVariant> it(availableColors);
    while (it.hasNext()) {
        it.next();
        QColor availableColor(it.key());
        VERIFY_OR_DEBUG_ASSERT(availableColor.isValid()) {
            qWarning() << "Received invalid color name from controller script:" << it.key();
            continue;
        }

        double distance = colorDistance(desiredColor, availableColor);
        if (distance < nearestColorDistance) {
            nearestColorDistance = distance;
            bool valueOk;
            nearestColorValue = it.value().toUInt(&valueOk);
            VERIFY_OR_DEBUG_ASSERT(availableColor.isValid()) {
                qWarning() << "Failed to convert value to uint:" << it.value();
            }
            qDebug() << "Match for " << desiredColor << " -> " << availableColor << "(distance =" << distance << ", value = " << nearestColorValue << ")";
        }
    }
    return nearestColorValue;
}

QScriptValue ColorJSProxy::makeHotcueColorPalette(QScriptEngine* pScriptEngine,
        HotcueColorPaletteSettings colorPaletteSettings) {
    // TODO: make sure we get notified when the palette changes
    QList<QColor> colorList =
            colorPaletteSettings.getHotcueColorPalette().m_colorList;
    int numColors = colorList.length();
    QScriptValue jsColorList = pScriptEngine->newArray(numColors);
    for (int i = 0; i < numColors; ++i) {
        QColor color = colorList.at(i);
        jsColorList.setProperty(i, colorFromHexCode(color.rgba()));
    }
    return jsColorList;
}
