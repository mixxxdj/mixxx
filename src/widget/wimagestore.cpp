#include "widget/wimagestore.h"

#include <QtDebug>
#include <QSvgRenderer>
#include <QPainter>

#include "skin/legacy/imgloader.h"

// static
QHash<ImageKey, std::weak_ptr<QImage>> WImageStore::m_dictionary;
std::shared_ptr<ImgSource> WImageStore::m_loader = std::make_shared<ImgLoader>();

// static
QImage* WImageStore::getImageNoCache(const QString& fileName, double scaleFactor) {
    return getImageNoCache(PixmapSource(fileName), scaleFactor);
}

// static
std::shared_ptr<QImage> WImageStore::getImage(const QString& fileName, double scaleFactor) {
    return getImage(PixmapSource(fileName), scaleFactor);
}

// static
std::shared_ptr<QImage> WImageStore::getImage(const PixmapSource& source, double scaleFactor) {
    if (source.isEmpty()) {
        return nullptr;
    }

    // Generate key struct
    ImageKey key{source.getPath(), scaleFactor};

    // Attempt to find the cached Image using the generated key.
    auto it = m_dictionary.find(key);
    if (it != m_dictionary.end()) {
        //qDebug() << "WImageStore returning cached Image for:" << source.getPath();
        return it.value().lock();
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << source.getPath();

    auto pLoadedImage = std::shared_ptr<QImage>(getImageNoCache(source, scaleFactor), &deleteImage);

    if (!pLoadedImage) {
        return nullptr;
    }

    if (pLoadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << source.getPath() << (pLoadedImage == nullptr);
        return nullptr;
    }

    m_dictionary.insert(key, pLoadedImage);
    return pLoadedImage;
}

// static
QImage* WImageStore::getImageNoCache(const PixmapSource& source, double scaleFactor) {
    if (source.isSVG()) {
        QSvgRenderer renderer;

        if (!source.getPath().isEmpty()) {
            if (!renderer.load(source.getPath())) {
                // The above line already logs a warning
                return nullptr;
            }
        } else {
            return nullptr;
        }

        auto* pImage = new QImage(renderer.defaultSize() * scaleFactor,
                QImage::Format_ARGB32_Premultiplied);
        pImage->fill(Qt::transparent);
        QPainter painter(pImage);
        renderer.render(&painter);
        return pImage;
    } else {
        return m_loader->getImage(source.getPath(), scaleFactor);
    }
}

// static
void WImageStore::deleteImage(QImage* p) {
    QMutableHashIterator<ImageKey, std::weak_ptr<QImage>> it(m_dictionary);
    while (it.hasNext()) {
        if(it.next().value().expired()) {
            it.remove();
        }
    }
    delete p;
}

// static
void WImageStore::correctImageColors(QImage* p) {
    m_loader->correctImageColors(p);
}

// static
bool WImageStore::willCorrectColors() {
    return m_loader->willCorrectColors();
};

// static
void WImageStore::setLoader(std::shared_ptr<ImgSource> ld) {
    m_loader = ld;
}
