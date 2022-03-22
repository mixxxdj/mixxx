#pragma once

#include <QByteArray>
#include <QColor>
#include <QImage>

namespace mixxx {

typedef QByteArray ImageDigest;

ImageDigest digestImage(const QImage& image);

QColor extractImageBackgroundColor(const QImage& image);

} // namespace mixxx
