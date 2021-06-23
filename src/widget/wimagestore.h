#pragma once

#include <QHash>
#include <QImage>
#include <QSharedPointer>
#include <unordered_map>

#include "skin/legacy/imgsource.h"
#include "skin/legacy/pixmapsource.h"
#include "util/memory.h"

class WImageStore {
  public:
    static std::shared_ptr<QImage> getImage(const QString& fileName, double scaleFactor);
    static QImage* getImageNoCache(const QString& fileName, double scaleFactor);
    static std::shared_ptr<QImage> getImage(const PixmapSource& source, double scaleFactor);
    static QImage* getImageNoCache(const PixmapSource& source, double scaleFactor);
    static void setLoader(QSharedPointer<ImgSource> ld);
    // For external owned images like software generated ones.
    static void correctImageColors(QImage* p);
    static bool willCorrectColors();

  private:
    static void deleteImage(QImage* p);

    // Dictionary of Images already instantiated
    static QHash<QString, std::weak_ptr<QImage> > m_dictionary;
    static QSharedPointer<ImgSource> m_loader;
};
