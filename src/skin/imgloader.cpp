#include <QApplication>
#include <QImageReader>
#include <QFileInfo>

#include "imgloader.h"
#include "widget/wwidget.h"

ImgLoader::ImgLoader() {
}

QImage* ImgLoader::getImage(const QString& fileName, double scaleFactor) const {
    QImage* pImage = new QImage();
    QFileInfo info(fileName);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    if (scaleFactor > 2.0) {
        // Try to load with @3x suffix
        QString strNewName = info.path() + "/" + info.baseName() + "@3x."
                + info.completeSuffix();
        QImageReader reader(info.fileName());
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scale = originalSize * (scaleFactor / 3.0);
            reader.setScaledSize(scale);
            reader.read(pImage);
            return pImage;
        }
    }
    if (scaleFactor > 1.0) {
#else
    if (qApp->devicePixelRatio() > 1.0) {
#endif
        // Try to load with @2x suffix
        QString strNewName = info.path() + "/" + info.baseName() + "@2x."
                + info.completeSuffix();
        QImageReader reader(info.fileName());
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scale = originalSize * (scaleFactor / 2.0);
            reader.setScaledSize(scale);
            reader.read(pImage);
            return pImage;
        }
    }

    {
        QImageReader reader(fileName);
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scale = originalSize * scaleFactor;
            reader.setScaledSize(scale);
            reader.read(pImage);
            return pImage;
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // No Image found, matching the desired resolution or lower.
    // try to load a bigger Images.

    {
        // Try to load with @2x suffix
        QString strNewName = info.path() + "/" + info.baseName() + "@2x."
                + info.completeSuffix();
        QImageReader reader(info.fileName());
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scale = originalSize * (scaleFactor / 2.0);
            reader.setScaledSize(scale);
            reader.read(pImage);
            return pImage;
        }
    }

    {
        // Try to load with @3x suffix
        QString strNewName = info.path() + "/" + info.baseName() + "@3x."
                + info.completeSuffix();
        QImageReader reader(info.fileName());
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scale = originalSize * (scaleFactor / 3.0);
            reader.setScaledSize(scale);
            reader.read(pImage);
            return pImage;
        }
    }
#endif
    return pImage;
}

