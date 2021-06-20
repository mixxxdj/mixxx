#include "util/imageutils.h"

#include <QCryptographicHash>

namespace {

constexpr QCryptographicHash::Algorithm kImageHashAlgorithm = QCryptographicHash::Sha256;

} // anonymous namespace

namespace mixxx {

ImageDigest digestImage(const QImage& image) {
    if (image.isNull()) {
        return ImageDigest();
    }
    QCryptographicHash cryptoHash(kImageHashAlgorithm);
    cryptoHash.addData(
            reinterpret_cast<const char*>(image.constBits()),
            image.sizeInBytes());
    return cryptoHash.result();
}

QColor extractImageBackgroundColor(const QImage& image) {
    if (image.size().isEmpty()) {
        // The average color of an empty image is undefined
        return QColor();
    }
    // Qt::SmoothTransformation is required for obtaining the average
    // color of the image! Otherwise the color of the single pixel
    // might just be sampled from a single original pixel.
    const auto singlePixelImage = image.scaled(
            QSize(1, 1),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
    return singlePixelImage.pixelColor(0, 0);
}

} // namespace mixxx
