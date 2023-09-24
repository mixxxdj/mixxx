#include "widget/wpixmapstore.h"

#include <QDir>
#include <QString>
#include <QtDebug>

#include "util/math.h"
#include "skin/legacy/imgloader.h"

// static
QHash<QString, WeakPaintablePointer> WPixmapStore::m_paintableCache;
QSharedPointer<ImgSource> WPixmapStore::m_loader
        = QSharedPointer<ImgSource>(new ImgLoader());

// static
PaintablePointer WPixmapStore::getPaintable(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    if (source.isEmpty()) {
        return PaintablePointer();
    }
    QString key = source.getId() + QString::number(mode) + QString::number(scaleFactor);

    // See if we have a cached value for the pixmap.
    PaintablePointer pPaintable = m_paintableCache.value(
            key,
            PaintablePointer());
    if (pPaintable) {
        return pPaintable;
    }

    pPaintable = PaintablePointer(new Paintable(source, mode, scaleFactor));

    m_paintableCache.insert(key, pPaintable);
    return pPaintable;
}

// static
QPixmap* WPixmapStore::getPixmapNoCache(
        const QString& fileName,
        double scaleFactor) {
    QPixmap* pPixmap = nullptr;
    QImage* img = m_loader->getImage(fileName, scaleFactor);
    pPixmap = new QPixmap();
    pPixmap->convertFromImage(*img);
    delete img;
    return pPixmap;
}

// static
void WPixmapStore::correctImageColors(QImage* p) {
    m_loader->correctImageColors(p);
}

bool WPixmapStore::willCorrectColors() {
    return m_loader->willCorrectColors();
};

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}
