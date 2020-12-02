/***************************************************************************
                          wpixmapstore.h  -  description
                             -------------------
    begin                : Mon Jun 28 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPIXMAPSTORE_H
#define WPIXMAPSTORE_H

#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QSvgRenderer>
#include <QWeakPointer>

#include "skin/imgsource.h"
#include "skin/pixmapsource.h"
#include "widget/paintable.h"

class ImgSource;
class QImage;
class QPixmap;
template<class Key, class T>
class QHash;
template<class T>
class QSharedPointer;

typedef QSharedPointer<Paintable> PaintablePointer;
typedef QWeakPointer<Paintable> WeakPaintablePointer;

class WPixmapStore {
  public:
    static PaintablePointer getPaintable(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    static QPixmap* getPixmapNoCache(const QString& fileName, double scaleFactor);
    static void setLoader(QSharedPointer<ImgSource> ld);
    static void correctImageColors(QImage* p);
    static bool willCorrectColors();

  private:
    static QHash<QString, WeakPaintablePointer> m_paintableCache;
    static QSharedPointer<ImgSource> m_loader;
};

#endif
