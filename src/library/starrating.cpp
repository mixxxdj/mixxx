/***************************************************************************
                           starrating.cpp
                              -------------------
     copyright            : (C) 2010 Tobias Rafreider
     copyright            : (C) 2009 Nokia Corporation

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "library/starrating.h"
#include "util/math.h"

// Magic number? Explain what this factor affects and how
const int PaintingScaleFactor = 15;

StarRating::StarRating(int starCount, int maxStarCount)
        : m_myStarCount(starCount),
          m_myMaxStarCount(maxStarCount) {
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
    return PaintingScaleFactor * QSize(m_myMaxStarCount, 1);
}

void StarRating::paint(QPainter *painter, const QRect &rect) const {
    // Assume the painter is configured with the right brush.
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    int yOffset = (rect.height() - PaintingScaleFactor) / 2;
    painter->translate(rect.x(), rect.y() + yOffset);
    painter->scale(PaintingScaleFactor, PaintingScaleFactor);

    //determine number of stars that are possible to paint
    int n = rect.width() / PaintingScaleFactor;

    for (int i = 0; i < m_myMaxStarCount && i<n; ++i) {
        if (i < m_myStarCount) {
            painter->drawPolygon(m_starPolygon, Qt::WindingFill);
        } else {
            painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
        }
        painter->translate(1.0, 0.0);
    }
}
