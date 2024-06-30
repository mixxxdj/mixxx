#pragma once

#include <QHash>
#include <memory>

#include "skin/legacy/pixmapsource.h"

class QImage;
class ImgSource;

struct ImageKey {
    QString path;
    double scaleFactor;

    bool operator==(const ImageKey& other) const {
        return path == other.path && scaleFactor == other.scaleFactor;
    }
};

namespace std {
template<>
struct hash<ImageKey> {
    std::size_t operator()(const ImageKey& key) const {
        return qHash(key.path) ^ std::hash<double>()(key.scaleFactor);
    }
};
} // namespace std

class WImageStore {
  public:
    static std::shared_ptr<QImage> getImage(const QString& fileName, double scaleFactor);
    static QImage* getImageNoCache(const QString& fileName, double scaleFactor);
    static std::shared_ptr<QImage> getImage(const PixmapSource& source, double scaleFactor);
    static QImage* getImageNoCache(const PixmapSource& source, double scaleFactor);
    static void setLoader(std::shared_ptr<ImgSource> ld);
    // For external owned images like software generated ones.
    static void correctImageColors(QImage* p);
    static bool willCorrectColors();

  private:
    static void deleteImage(QImage* p);

    // Dictionary of Images already instantiated
    static QHash<ImageKey, std::weak_ptr<QImage>> m_dictionary;
    static std::shared_ptr<ImgSource> m_loader;
};
