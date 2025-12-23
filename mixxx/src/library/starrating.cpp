#include "library/starrating.h"

#include <QPainter>
#include <QRect>

#include "util/math.h"
#include "util/painterscope.h"

// Magic number? Explain what this factor affects and how
constexpr int PaintingScaleFactor = 15;

StarRating::StarRating(
        int starCount,
        int maxStarCount)
        : m_starCount(starCount),
          m_maxStarCount(maxStarCount) {
    DEBUG_ASSERT(verifyStarCount(m_starCount));
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
    return PaintingScaleFactor * QSize(m_maxStarCount, 1);
}

void StarRating::paint(QPainter* painter, const QRect& rect) const {
    PainterScope painterScope(painter);
    // Assume the painter is configured with the right brush.
    painter->setRenderHint(QPainter::Antialiasing, true);
    // Don't draw outlines, only fill the polygons
    painter->setPen(Qt::NoPen);

    // Center vertically inside the table cell, and also center horizontally
    // if the cell is wider than the minimum stars width.
    int xOffset = std::max((rect.width() - sizeHint().width()) / 2, 0);
    int yOffset = (rect.height() - PaintingScaleFactor) / 2;
    painter->translate(rect.x() + xOffset, rect.y() + yOffset);
    painter->scale(PaintingScaleFactor, PaintingScaleFactor);

    // Determine number of stars that are possible to paint
    int n = rect.width() / PaintingScaleFactor;

    for (int i = 0; i < m_maxStarCount && i < n; ++i) {
        if (i < m_starCount) {
            painter->drawPolygon(m_starPolygon, Qt::WindingFill);
        } else {
            painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
        }
        painter->translate(1.0, 0.0);
    }
}

int StarRating::starAtPosition(int x, const QRect& rect) const {
    // The star rating is drawn centered in the parent (WStarrating or
    // cell of StarDelegate, so we need to shift the x input as well.
    int starsWidth = sizeHint().width();
    int xOffset = std::max((rect.width() - starsWidth) / 2, 0);
    // Only shift if the parent is wider than the star rating
    x -= std::max(xOffset, 0);

    // Return invalid if the pointer left the star rectangle at either side.
    // If the the parent is wider than the star rating, add a half star margin
    // at the left to simplify setting 0.
    double leftVoid = xOffset > starsWidth * 0.05 ? starsWidth * -0.05 : 0;
    if (x < leftVoid || x >= starsWidth) {
        return StarRating::kInvalidStarCount;
    } else if (x < starsWidth * 0.05) {
        // If the pointer is very close to the left edge, set 0 stars.
        return 0;
    }

    int star = (x / (starsWidth / maxStarCount())) + 1;

    if (star <= 0 || star > maxStarCount()) {
        return 0;
    }
    return star;
}
