#include "widget/wimagestore.h"

#include <QtDebug>
#include <QSvgRenderer>
#include <QPainter>

#include "skin/legacy/imgloader.h"


// static
QHash<QString, std::shared_ptr<QImage>> WImageStore::m_dictionary;
QSharedPointer<ImgSource> WImageStore::m_loader = QSharedPointer<ImgSource>(new ImgLoader());

// static
std::shared_ptr<QImage> WImageStore::getImage(const QString& fileName, double scaleFactor) {
    return getImage(PixmapSource(fileName), scaleFactor);
}

// static
std::shared_ptr<QImage> WImageStore::getImage(const PixmapSource& source, double scaleFactor) {
    if (source.isEmpty()) {
        return nullptr;
    }
    // Search for Image in list
    QString key = source.getId() + QString::number(scaleFactor);

    auto it = m_dictionary.find(key);
    if (it != m_dictionary.end()) {
        return it.value();
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << source.getPath();

    auto pLoadedImage = std::make_shared<QImage>(*getImageNoCache(source, scaleFactor));

    if (!pLoadedImage || pLoadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << source.getPath();
        return nullptr;
    }

    m_dictionary[key] = pLoadedImage;
    return pLoadedImage;
}

// static
std::unique_ptr<QImage> WImageStore::getImageNoCache(
        const PixmapSource& source, double scaleFactor) {
    if (source.isSVG()) {
        QSvgRenderer renderer;
        std::unique_ptr<QImage> pImage;

        // Attempt to load from SVG source data if available
        if (!source.getSvgSourceData().isEmpty() && renderer.load(source.getSvgSourceData())) {
            pImage = std::make_unique<QImage>(
                    renderer.defaultSize() * scaleFactor,
                    QImage::Format_ARGB32);
        }
        // If loading from source data fails or isn't available, try loading from path
        else if (renderer.load(source.getPath())) {
            pImage = std::make_unique<QImage>(
                    renderer.defaultSize() * scaleFactor,
                    QImage::Format_ARGB32);
        }

        // If an image was successfully created, render the SVG into it
        if (pImage) {
            pImage->fill(Qt::transparent);
            QPainter painter(pImage.get());
            renderer.render(&painter);
            return pImage;
        }
    } else {
        // For non-SVG, delegate to the loader
        return std::unique_ptr<QImage>(m_loader->getImage(source.getPath(), scaleFactor));
    }
    return nullptr;
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
void WImageStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;
}
