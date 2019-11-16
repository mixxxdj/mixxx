#include "controllers/colormapper.h"

#include <cmath>
#include <cstdint>

#include "util/debug.h"

namespace {
double colorDistance(QRgb a, QRgb b) {
    // This algorithm calculates the distance between two colors. In
    // contrast to the L2 norm, this also tries take the human perception
    // of colors into account. More accurate algorithms like the CIELAB2000
    // Delta-E rely on sophisticated color space conversions and need a lot
    // of costly computations. In contrast, this is a low-cost
    // approximation and should be sufficently accurate.
    // More details: https://www.compuphase.com/cmetric.htm
    long mean_red = ((long)qRed(a) + (long)qRed(b)) / 2;
    long delta_red = (long)qRed(a) - (long)qRed(b);
    long delta_green = (long)qGreen(a) - (long)qGreen(b);
    long delta_blue = (long)qBlue(a) - (long)qBlue(b);
    return sqrt(
            (((512 + mean_red) * delta_red * delta_red) >> 8) +
            (4 * delta_green * delta_green) +
            (((767 - mean_red) * delta_blue * delta_blue) >> 8));
}
} // namespace

QPair<QRgb, QVariant> ColorMapper::getNearestColor(QRgb desiredColor) {
    // If desired color is already in cache, use cache entry
    QMap<QRgb, QRgb>::const_iterator i = m_cache.find(desiredColor);
    QMap<QRgb, QVariant>::const_iterator j;
    if (i != m_cache.constEnd()) {
        j = m_availableColors.find(i.value());
        DEBUG_ASSERT(j != m_availableColors.constEnd());
        qDebug() << "ColorMapper cache hit for" << desiredColor << ":"
                 << "Color =" << j.key() << ","
                 << "Value =" << j.value();
        return QPair<QRgb, QVariant>(j.key(), j.value());
    }

    // Color is not cached
    QMap<QRgb, QVariant>::const_iterator nearestColorIterator;
    double nearestColorDistance = qInf();
    for (j = m_availableColors.constBegin(); j != m_availableColors.constEnd(); j++) {
        QRgb availableColor = j.key();
        double distance = colorDistance(desiredColor, availableColor);
        if (distance < nearestColorDistance) {
            nearestColorDistance = distance;
            nearestColorIterator = j;
        }
    }

    DEBUG_ASSERT(nearestColorDistance < qInf());
    qDebug() << "ColorMapper found matching color for" << desiredColor << ":"
             << "Color =" << nearestColorIterator.key() << ","
             << "Value =" << nearestColorIterator.value();
    m_cache.insert(desiredColor, nearestColorIterator.key());
    return QPair<QRgb, QVariant>(nearestColorIterator.key(), nearestColorIterator.value());
}
