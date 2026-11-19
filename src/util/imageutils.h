#pragma once

#include <QByteArray>
#include <QColor>
#include <QImage>

namespace mixxx {

typedef QByteArray ImageDigest;

ImageDigest digestImage(const QImage& image);

QColor extractImageBackgroundColor(const QImage& image);

/// Returns a small, blurred and opaque copy of the image that keeps its
/// aspect ratio, or a null image if the image is null. The copy is at most
/// maxSize pixels wide and high and is blurred with a radius of blurRadius
/// of its pixels. Drawn enlarged with linear filtering it serves as a soft
/// backdrop.
QImage blurredThumbnail(const QImage& image, int maxSize, int blurRadius);

} // namespace mixxx
