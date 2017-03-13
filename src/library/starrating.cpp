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

#include "library/starrating.h"
#include "util/math.h"

const int PaintingScaleFactor = 15;

StarRating::StarRating(int starCount, int maxStarCount)
        : m_myStarCount(starCount),
          m_myMaxStarCount(maxStarCount) {
    m_starPolygon << QPointF(1.0, 0.5);
    for (int i = 1; i < 5; ++i)
        m_starPolygon << QPointF(0.5 + 0.5 * cos(0.8 * i * 3.14), 0.5 + 0.5 * sin(0.8 * i * 3.14));
    m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);
}

QSize StarRating::sizeHint() const {
    return PaintingScaleFactor * QSize(m_myMaxStarCount, 1);
}

void StarRating::paint(QPainter *painter, const QRect &rect) const {
    // Assume the painter is configured with the right brush.
    painter->save();
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
    painter->restore();
}
