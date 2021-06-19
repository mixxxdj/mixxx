#include <QtDebug>
#include <QCryptographicHash>

#include "skin/legacy/pixmapsource.h"
#include "util/assert.h"

PixmapSource::PixmapSource()
     : m_eType(SVG) {
}

PixmapSource::PixmapSource(const QString& filepath)
    : m_path(filepath) {
    if (m_path.endsWith(".svg", Qt::CaseInsensitive)) {
        m_eType = SVG;
    } else {
        m_eType = BITMAP;
    }
}

const QByteArray& PixmapSource::getSvgSourceData() const {
    return m_svgSourceData;
}

const QString& PixmapSource::getPath() const {
    return m_path;
}

bool PixmapSource::isEmpty() const {
    return m_path.isEmpty() && m_svgSourceData.isEmpty() ;
}

bool PixmapSource::isSVG() const {
    return m_eType == SVG;
}

bool PixmapSource::isBitmap() const {
    return m_eType == BITMAP;
}

void PixmapSource::setSVG(const QByteArray& content) {
    m_svgSourceData = content;
    m_eType = SVG;
}

QString PixmapSource::getId() const {
    if (m_svgSourceData.isEmpty()) {
        DEBUG_ASSERT(!m_path.isEmpty());
        return m_path;
    }
    return QCryptographicHash::hash(m_svgSourceData, QCryptographicHash::Sha1).toHex();
}
