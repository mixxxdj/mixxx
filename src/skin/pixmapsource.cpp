#include <QtDebug>

#include "skin/pixmapsource.h"

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

QByteArray PixmapSource::getData() const {
    return m_svgSourceData;
}

QString PixmapSource::getPath() const {
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
    quint16 checksum;
    if (m_svgSourceData.isEmpty()) {
        checksum = qChecksum(m_path.toAscii().constData(), m_path.length());
    } else {
        checksum = qChecksum(m_svgSourceData.constData(), m_svgSourceData.length());
    }
    return m_path + QString::number(checksum);
}
