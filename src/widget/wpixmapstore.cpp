#include "widget/wpixmapstore.h"

#include "skin/legacy/imgloader.h"
#include "widget/paintable.h"

// static
QHash<PixmapKey, WeakPaintablePointer> WPixmapStore::m_paintableCache;
std::shared_ptr<ImgSource> WPixmapStore::m_loader = std::make_shared<ImgLoader>();

// static
PaintablePointer WPixmapStore::getPaintable(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    if (source.isEmpty()) {
        return PaintablePointer();
    }
    // Generate a key struct:
    PixmapKey key{source.getPath(), mode, scaleFactor};

    // Attempt to find the cached Paintable using the generated key.
    auto it = m_paintableCache.find(key);
    if (it != m_paintableCache.end()) {
        return it.value().lock();
    }

    // If not found, create a new Paintable, cache it, and return it.
    PaintablePointer pPaintable = std::make_shared<Paintable>(source, mode, scaleFactor);
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

void WPixmapStore::setLoader(std::shared_ptr<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_paintableCache.clear();
}
