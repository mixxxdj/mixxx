#include "widget/wpixmapstore.h"

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
    QString key = QStringLiteral("%1%2%3").arg(source.getId(),
            QString::number(mode),
            QString::number(scaleFactor));

    // See if we have a cached value for the pixmap.
    auto it = m_paintableCache.find(key);
    if (it != m_paintableCache.end()) {
        return it.value();
    }

    PaintablePointer pPaintable(new Paintable(source, mode, scaleFactor));
    m_paintableCache.insert(key, pPaintable);
    return pPaintable;
}

// static
std::unique_ptr<QPixmap> WPixmapStore::getPixmapNoCache(
        const QString& fileName,
        double scaleFactor) {
    auto* pImage = m_loader->getImage(fileName, scaleFactor);
    if (!pImage || pImage->isNull()) {
        return nullptr;
    }
    return std::make_unique<QPixmap>(QPixmap::fromImage(*pImage));
}

// static
void WPixmapStore::correctImageColors(QImage* p) {
    m_loader->correctImageColors(p);
}

bool WPixmapStore::willCorrectColors() {
    return m_loader->willCorrectColors();
}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}
