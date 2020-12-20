#include "controllers/scripting/colormapper.h"

#include <cmath>
#include <cstdint>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("ColorMapper");

double colorDistance(QRgb a, QRgb b) {
    // This algorithm calculates the distance between two colors. In
    // contrast to the L2 norm, this also tries take the human perception
    // of colors into account. More accurate algorithms like the CIELAB2000
    // Delta-E rely on sophisticated color space conversions and need a lot
    // of costly computations. In contrast, this is a low-cost
    // approximation and should be sufficiently accurate.
    // More details: https://www.compuphase.com/cmetric.htm
    long mean_red = (static_cast<long>(qRed(a)) + static_cast<long>(qRed(b))) / 2;
    long delta_red = static_cast<long>(qRed(a)) - static_cast<long>(qRed(b));
    long delta_green = static_cast<long>(qGreen(a)) - static_cast<long>(qGreen(b));
    long delta_blue = static_cast<long>(qBlue(a)) - static_cast<long>(qBlue(b));
    return sqrt(
            (((512 + mean_red) * delta_red * delta_red) >> 8) +
            (4 * delta_green * delta_green) +
            (((767 - mean_red) * delta_blue * delta_blue) >> 8));
}

} // namespace

QRgb ColorMapper::getNearestColor(QRgb desiredColor) {
    // If desired color is already in cache, use cache entry
    const auto iCachedColor = m_cache.constFind(desiredColor);
    if (iCachedColor != m_cache.constEnd()) {
        const QRgb cachedColor = iCachedColor.value();
        DEBUG_ASSERT(m_availableColors.contains(cachedColor));
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Found cached color"
                    << cachedColor
                    << "for"
                    << desiredColor;
        }
        return cachedColor;
    }

    // Color is not cached
    auto iNearestColor = m_availableColors.constEnd();
    double nearestColorDistance = qInf();
    for (auto i = m_availableColors.constBegin(); i != m_availableColors.constEnd(); ++i) {
        const QRgb availableColor = i.key();
        double distance = colorDistance(desiredColor, availableColor);
        if (distance < nearestColorDistance) {
            nearestColorDistance = distance;
            iNearestColor = i;
        }
    }
    if (iNearestColor != m_availableColors.constEnd()) {
        const QRgb nearestColor = iNearestColor.key();
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Found matching color"
                    << nearestColor
                    << "for"
                    << desiredColor;
        }
        m_cache.insert(desiredColor, nearestColor);
        return nearestColor;
    }

    DEBUG_ASSERT(m_availableColors.isEmpty());
    DEBUG_ASSERT(!"Unreachable: No matching color found");
    return desiredColor;
}

QVariant ColorMapper::getValueForNearestColor(QRgb desiredColor) {
    return m_availableColors.value(getNearestColor(desiredColor));
}
