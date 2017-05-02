#include "widget/wpixmapstore.h"

#include <QDir>
#include <QString>
#include <QtDebug>

#include "util/math.h"
#include "skin/imgloader.h"

// static
QHash<QString, WeakPaintablePointer> WPixmapStore::m_paintableCache;
QSharedPointer<ImgSource> WPixmapStore::m_loader
        = QSharedPointer<ImgSource>(new ImgLoader());

// static
PaintablePointer WPixmapStore::getPaintable(PixmapSource source,
                                            Paintable::DrawMode mode,
                                            double scaleFactor) {
    QString key = source.getId() + QString::number(mode) + QString::number(scaleFactor);

    // See if we have a cached value for the pixmap.
    PaintablePointer pPaintable = m_paintableCache.value(
            key,
            PaintablePointer());
    if (pPaintable) {
        return pPaintable;
    }

    // Otherwise, construct it with the pixmap loader.
    //qDebug() << "WPixmapStore Loading pixmap from file" << source.getPath();

    if (mode == Paintable::FIXED || mode == Paintable::TILE || !source.isSVG()) {
        QImage* pImage = m_loader->getImage(source.getPath(), scaleFactor);
        pPaintable = PaintablePointer(new Paintable(pImage, mode));
    } else {
        pPaintable = PaintablePointer(new Paintable(source, mode));
    }

    if (pPaintable->isNull()) {
        // Only log if it looks like the user tried to specify a
        // pixmap. Otherwise we probably just have a widget that is calling
        // getPaintable without checking that the skinner actually wanted one.
        if (!source.isEmpty()) {
            qDebug() << "WPixmapStore couldn't load:" << source.getPath()
                     << pPaintable.isNull();
        }
        return PaintablePointer();
    }

    m_paintableCache.insert(key, pPaintable);
    return pPaintable;
}

// static
QPixmap* WPixmapStore::getPixmapNoCache(
        const QString& fileName,
        double scaleFactor) {
    QPixmap* pPixmap = nullptr;
    QImage* img = m_loader->getImage(fileName, scaleFactor);
#if QT_VERSION >= 0x040700
    pPixmap = new QPixmap();
    pPixmap->convertFromImage(*img);
#else
    pPixmap = new QPixmap(QPixmap::fromImage(*img));
#endif
    delete img;
    return pPixmap;
}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}
