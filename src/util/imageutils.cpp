#include "util/imageutils.h"

#include <QCryptographicHash>
#include <algorithm>

#include "util/assert.h"

namespace {

constexpr QCryptographicHash::Algorithm kImageHashAlgorithm = QCryptographicHash::Sha256;

// Three passes of a box blur approximate a Gaussian blur.
constexpr int kBlurPasses = 3;

// Averages every pixel with its neighbors within the radius along one axis.
// The step is (1, 0) for the horizontal and (0, 1) for the vertical axis.
// The image must have the format RGB32. Its pixels are accessed directly,
// which is much faster than QImage::pixel().
QImage boxBlurred(const QImage& source, int radius, int stepX, int stepY) {
    const int kernelSize = 2 * radius + 1;
    const int width = source.width();
    const int height = source.height();
    QImage blurred(source.size(), QImage::Format_RGB32);
    const int sourceStride = static_cast<int>(source.bytesPerLine() / sizeof(QRgb));
    const int blurredStride = static_cast<int>(blurred.bytesPerLine() / sizeof(QRgb));
    const QRgb* pSource = reinterpret_cast<const QRgb*>(source.constBits());
    QRgb* pBlurred = reinterpret_cast<QRgb*>(blurred.bits());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int red = 0;
            int green = 0;
            int blue = 0;
            for (int offset = -radius; offset <= radius; ++offset) {
                const int sampleX = std::clamp(x + offset * stepX, 0, width - 1);
                const int sampleY = std::clamp(y + offset * stepY, 0, height - 1);
                const QRgb sample = pSource[sampleY * sourceStride + sampleX];
                red += qRed(sample);
                green += qGreen(sample);
                blue += qBlue(sample);
            }
            pBlurred[y * blurredStride + x] =
                    qRgb(red / kernelSize, green / kernelSize, blue / kernelSize);
        }
    }
    return blurred;
}

} // anonymous namespace

namespace mixxx {

ImageDigest digestImage(const QImage& image) {
    if (image.isNull()) {
        return ImageDigest();
    }
    QCryptographicHash cryptoHash(kImageHashAlgorithm);
    const QByteArray data = QByteArray::fromRawData(
            reinterpret_cast<const char*>(image.constBits()),
            image.sizeInBytes());
    cryptoHash.addData(data);
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

QImage blurredThumbnail(const QImage& image, int maxSize, int blurRadius) {
    VERIFY_OR_DEBUG_ASSERT(maxSize > 0 && blurRadius >= 0) {
        return QImage();
    }
    if (image.isNull()) {
        return QImage();
    }
    QImage blurred = image.scaled(maxSize,
                                  maxSize,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation)
                             .convertToFormat(QImage::Format_RGB32);
    for (int pass = 0; pass < kBlurPasses; ++pass) {
        blurred = boxBlurred(boxBlurred(blurred, blurRadius, 1, 0), blurRadius, 0, 1);
    }
    return blurred;
}

} // namespace mixxx
