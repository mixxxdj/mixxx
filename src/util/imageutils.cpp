#include "util/imageutils.h"

#include <QCryptographicHash>
#include <algorithm>

namespace {

constexpr QCryptographicHash::Algorithm kImageHashAlgorithm = QCryptographicHash::Sha256;

constexpr int kBlurredThumbnailSize = 64;
constexpr int kBlurRadius = 5;
// [AI-generated] Three passes of a box blur approximate a Gaussian blur.
// [/AI-generated]
constexpr int kBlurPasses = 3;

// [AI-generated] Averages every pixel with its neighbors along one axis. The
// step is (1, 0) for the horizontal and (0, 1) for the vertical axis. The
// image must have the format RGB32. Its pixels are accessed directly, which
// is much faster than QImage::pixel(). [/AI-generated]
QImage boxBlurred(const QImage& source, int stepX, int stepY) {
    constexpr int kKernelSize = 2 * kBlurRadius + 1;
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
            for (int offset = -kBlurRadius; offset <= kBlurRadius; ++offset) {
                const int sampleX = std::clamp(x + offset * stepX, 0, width - 1);
                const int sampleY = std::clamp(y + offset * stepY, 0, height - 1);
                const QRgb sample = pSource[sampleY * sourceStride + sampleX];
                red += qRed(sample);
                green += qGreen(sample);
                blue += qBlue(sample);
            }
            pBlurred[y * blurredStride + x] =
                    qRgb(red / kKernelSize, green / kKernelSize, blue / kKernelSize);
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

QImage blurredThumbnail(const QImage& image) {
    if (image.isNull()) {
        return QImage();
    }
    QImage blurred = image.scaled(kBlurredThumbnailSize,
                                  kBlurredThumbnailSize,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation)
                             .convertToFormat(QImage::Format_RGB32);
    for (int pass = 0; pass < kBlurPasses; ++pass) {
        blurred = boxBlurred(boxBlurred(blurred, 1, 0), 0, 1);
    }
    return blurred;
}

} // namespace mixxx
