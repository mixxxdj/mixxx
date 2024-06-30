#pragma once

#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QString>
#include <QSvgRenderer>
#include <memory>

#include "skin/legacy/imgsource.h"
#include "skin/legacy/pixmapsource.h"
#include "widget/paintable.h"

using PaintablePointer = std::shared_ptr<Paintable>;
using WeakPaintablePointer = std::weak_ptr<Paintable>;
class WPixmapStore {
  public:
    static PaintablePointer getPaintable(
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    static std::unique_ptr<QPixmap> getPixmapNoCache(
            const QString& fileName,
            double scaleFactor);
    static void setLoader(std::shared_ptr<ImgSource> ld);
    static void correctImageColors(QImage* p);
    static bool willCorrectColors();

  private:
    static QHash<QString, WeakPaintablePointer> m_paintableCache;
    static std::shared_ptr<ImgSource> m_loader;
};
