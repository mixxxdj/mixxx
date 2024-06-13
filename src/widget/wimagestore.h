#pragma once

#include <QHash>
#include <QSharedPointer>
#include <memory>

#include "skin/legacy/pixmapsource.h"

class QImage;
class ImgSource;

class WImageStore {
  public:
    static std::shared_ptr<QImage> getImage(const QString& fileName, double scaleFactor);
    static std::shared_ptr<QImage> getImage(const PixmapSource& source, double scaleFactor);
    static std::unique_ptr<QImage> getImageNoCache(const PixmapSource& source, double scaleFactor);
    static void setLoader(QSharedPointer<ImgSource> ld);
    // For external owned images like software generated ones.
    static void correctImageColors(QImage* p);
    static bool willCorrectColors();

  private:

    // Dictionary of Images already instantiated
    static QHash<QString, std::shared_ptr<QImage>> m_dictionary;
    static QSharedPointer<ImgSource> m_loader;
};
