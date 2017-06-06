#include "widget/wpixmapstore.h"

#include <QIcon>
#include <QDir>
#include <QString>
#include <QtDebug>

#include "util/math.h"
#include "skin/imgloader.h"

// static
QHash<QString, WeakPaintablePointer> WPixmapStore::m_paintableCache;
QSharedPointer<ImgSource> WPixmapStore::m_pLoader
        = QSharedPointer<ImgSource>(new ImgLoader());
QSharedPointer<ImgSource> WPixmapStore::m_pIconLoader = QSharedPointer<ImgSource>();

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

    pPaintable = PaintablePointer(new Paintable(source, mode, scaleFactor));

    m_paintableCache.insert(key, pPaintable);
    return pPaintable;
}

// static
QPixmap* WPixmapStore::getPixmapNoCache(
        const QString& fileName,
        double scaleFactor) {
    QPixmap* pPixmap = nullptr;
    QImage* img = m_pLoader->getImage(fileName, scaleFactor);
#if QT_VERSION >= 0x040700
    pPixmap = new QPixmap();
    pPixmap->convertFromImage(*img);
#else
    pPixmap = new QPixmap(QPixmap::fromImage(*img));
#endif
    delete img;
    return pPixmap;
}

// static
void WPixmapStore::correctImageColors(QImage* p) {
    m_pLoader->correctImageColors(p);
}

bool WPixmapStore::willCorrectColors() {
    return m_pLoader->willCorrectColors();
};

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_pLoader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}

void WPixmapStore::setLibraryIconLoader(QSharedPointer<ImgSource> pIconLoader) {
    m_pIconLoader = pIconLoader;
}

QIcon WPixmapStore::getLibraryIcon(const QString& fileName) {
    return QIcon(getLibraryPixmap(fileName));
}

QPixmap WPixmapStore::getLibraryPixmap(const QString& fileName) {
    if (m_pIconLoader.isNull()) {
        return QPixmap(fileName);
    }

    QImage* image = m_pIconLoader->getImage(fileName, 1.0);

    if (!m_pLoader.isNull()) {
        m_pLoader->correctImageColors(image);

    }

    QPixmap pixmap(QPixmap::fromImage(*image));
    delete image;
    return pixmap;
}
