#include "widget/wimagestore.h"

#include <QtDebug>
#include <QSvgRenderer>
#include <QPainter>

#include "skin/imgloader.h"
#include "util/assert.h"

// static
QHash<QString, WImageStore::ImageInfoType*> WImageStore::m_dictionary;
QSharedPointer<ImgSource> WImageStore::m_loader
        = QSharedPointer<ImgSource>(new ImgLoader());

// static
QImage* WImageStore::getImageNoCache(const QString& fileName, double scaleFactor) {
    return getImageNoCache(PixmapSource(fileName), scaleFactor);
}

// static
QImage* WImageStore::getImage(const QString& fileName, double scaleFactor) {
    return getImage(PixmapSource(fileName), scaleFactor);
}

// static
QImage* WImageStore::getImage(const PixmapSource& source, double scaleFactor) {
    if (source.isEmpty()) {
        return nullptr;
    }
    // Search for Image in list
    ImageInfoType* info = nullptr;
    QString key = source.getId() + QString::number(scaleFactor);

    QHash<QString, ImageInfoType*>::iterator it = m_dictionary.find(key);
    if (it != m_dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WImageStore returning cached Image for:" << source.getPath();
        return info->image;
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << source.getPath();

    QImage* pLoadedImage = getImageNoCache(source, scaleFactor);

    if (pLoadedImage == nullptr) {
        return nullptr;
    }

    if (pLoadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << source.getPath() << (pLoadedImage == nullptr);
        delete pLoadedImage;
        return nullptr;
    }

    info = new ImageInfoType;
    info->image = pLoadedImage;
    info->instCount = 1;
    m_dictionary.insert(key, info);
    return info->image;
}

// static
QImage* WImageStore::getImageNoCache(const PixmapSource& source, double scaleFactor) {
    if (source.isSVG()) {
        QSvgRenderer renderer;

        if (!source.getSvgSourceData().isEmpty()) {
            // Call here the different overload for svg content
            if (!renderer.load(source.getSvgSourceData())) {
                // The above line already logs a warning
                return nullptr;
            }
        } else if (!source.getPath().isEmpty()) {
            if (!renderer.load(source.getPath())) {
                // The above line already logs a warning
                return nullptr;
            }
        } else {
            return nullptr;
        }

        auto pImage = new QImage(renderer.defaultSize() * scaleFactor,
                            QImage::Format_ARGB32);
        pImage->fill(0x00000000);  // Transparent black.
        QPainter painter(pImage);
        renderer.render(&painter);
        return pImage;
    } else {
        return m_loader->getImage(source.getPath(), scaleFactor);
    }
}

// static
void WImageStore::deleteImage(QImage* p)
{
    // Search for Image in list
    ImageInfoType *info = nullptr;
    QMutableHashIterator<QString, ImageInfoType*> it(m_dictionary);

    while (it.hasNext())
    {
        info = it.next().value();
        if (p == info->image)
        {
            info->instCount--;
            if (info->instCount<1)
            {
                it.remove();
                delete info->image;
                delete info;
            }
            break;
        }
    }
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
