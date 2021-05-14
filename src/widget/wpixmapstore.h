#pragma once

#include <QPixmap>
#include <QHash>
#include <QSharedPointer>
#include <QSvgRenderer>
#include <QImage>
#include <QScopedPointer>
#include <QPainter>
#include <QRectF>
#include <QString>

#include "skin/imgsource.h"
#include "skin/pixmapsource.h"
#include "widget/paintable.h"


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
