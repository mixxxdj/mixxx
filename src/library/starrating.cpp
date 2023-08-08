#include "library/starrating.h"

#include <QPainter>
#include <QRect>

#include "util/math.h"

// Default width/height of the rectangle for painting one star/diamond polygon
constexpr int kDefaultPaintingScaleFactor = 15;

StarRating::StarRating(
        int starCount,
        int maxStarCount)
        : m_starCount(starCount),
          m_maxStarCount(maxStarCount),
          m_scaleFactor(kDefaultPaintingScaleFactor) {
    DEBUG_ASSERT(m_starCount >= kMinStarCount);
    DEBUG_ASSERT(m_starCount <= m_maxStarCount);
    // 1st star cusp at 0° of the unit circle whose center is shifted to adapt the 0,0-based paint area
    m_starPolygon << QPointF(1.0, 0.5);
    for (int i = 1; i < 5; ++i) {
        // add QPointF 2-5 to polygon point array, equally distributed on a circumference.
        // To create a star (not a pentagon) we need to connect every second of those points.
        // This should actually give us a star that points up, but the drawn result points right.
        // x-y axes are swapped?
        m_starPolygon << QPointF(0.5 + 0.5 * cos(0.8 * i * 3.14), 0.5 + 0.5 * sin(0.8 * i * 3.14));
    }
    // creates 5 points for a tiny diamond/rhombe (square turned by 45°)
    // why do we need 5 cusps here? 4 should suffice as the polygon is closed automatically for the star above..
    m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);
}

QSize StarRating::sizeHint() const {
    return m_scaleFactor * getNormalizedSize();
}

void StarRating::paint(QPainter* painter, const QRect& rect, int height) {
    m_scaleFactor = height > 0 ? height : kDefaultPaintingScaleFactor;
    // Assume the painter is configured with the right brush.
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    // Shift the painter rectangle to center the polygons vertically
    int yOffset = (rect.height() - m_scaleFactor) / 2;
    painter->translate(rect.x(), rect.y() + yOffset);
    painter->scale(m_scaleFactor, m_scaleFactor);

    // Determine number of stars that are possible to paint
    int n = rect.width() / m_scaleFactor;

    for (int i = 0; i < m_maxStarCount && i < n; ++i) {
        if (i < m_starCount) {
            painter->drawPolygon(m_starPolygon, Qt::WindingFill);
        } else {
            painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
        }
        painter->translate(1.0, 0.0);
    }
}
