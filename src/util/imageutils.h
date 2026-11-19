#pragma once

#include <QByteArray>
#include <QColor>
#include <QImage>

namespace mixxx {

typedef QByteArray ImageDigest;

ImageDigest digestImage(const QImage& image);

QColor extractImageBackgroundColor(const QImage& image);

/// Returns a small, heavily blurred and opaque copy of the
/// image that keeps its aspect ratio, or a null image if the image is null.
/// Drawn enlarged with linear filtering it serves as a soft backdrop.
QImage blurredThumbnail(const QImage& image);

} // namespace mixxx
