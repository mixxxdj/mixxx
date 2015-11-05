#include <QtDebug>

#include "skin/pixmapsource.h"

PixmapSource::PixmapSource() {
}

PixmapSource::PixmapSource(const QString& filepath) {
    setPath(filepath);
}

PixmapSource::~PixmapSource() {
}

QByteArray PixmapSource::getData() const {
    return m_baData;
}

QString PixmapSource::getPath() const {
    return m_path;
}

void PixmapSource::setPath(const QString& newPath) {
    m_baData.truncate(0);
    m_path = newPath;
    if (m_path.endsWith(".svg", Qt::CaseInsensitive)) {
        m_eType = SVG;
    } else {
        m_eType = BITMAP;
    }
}

bool PixmapSource::isEmpty() const {
    return m_path.isEmpty() && m_baData.isEmpty() ;
}

bool PixmapSource::isSVG() const {
    return m_eType == SVG;
}

bool PixmapSource::isBitmap() const {
    return m_eType == BITMAP;
}

void PixmapSource::setSVG(const QByteArray& content) {
    m_baData = content;
    m_eType = SVG;
}

QString PixmapSource::getId() const {
    quint16 checksum;
    if (m_baData.isEmpty()) {
        checksum = qChecksum(m_path.toAscii().constData(), m_path.length());
    } else {
        checksum = qChecksum(m_baData.constData(), m_baData.length());
    }
    return m_path + QString::number(checksum);
}
