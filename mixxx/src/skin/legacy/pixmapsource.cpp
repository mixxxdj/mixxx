#include "skin/legacy/pixmapsource.h"

#include <QCryptographicHash>

#include "util/assert.h"

PixmapSource::PixmapSource()
        : m_eType(Type::SVG) {
}

PixmapSource::PixmapSource(const QString& filepath)
    : m_path(filepath) {
    if (m_path.endsWith(".svg", Qt::CaseInsensitive)) {
        m_eType = Type::SVG;
    } else {
        m_eType = Type::BITMAP;
    }
}

const QString& PixmapSource::getPath() const {
    DEBUG_ASSERT(!m_path.isEmpty());
    return m_path;
}

bool PixmapSource::isEmpty() const {
    return m_path.isEmpty();
}

bool PixmapSource::isSVG() const {
    return m_eType == Type::SVG;
}

bool PixmapSource::isBitmap() const {
    return m_eType == Type::BITMAP;
}
