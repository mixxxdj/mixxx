#include <QtDebug>

#include "skin/pixmapsource.h"

PixmapSource::PixmapSource() {
}

PixmapSource::~PixmapSource() {
}

QByteArray PixmapSource::getData() const {
    return m_baData;
}

QString PixmapSource::getPath() const {
    return m_path;
}

void PixmapSource::setPath( QString newPath ) {
    m_path = newPath;
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

void PixmapSource::setSVG( QByteArray content ) {
    m_baData.truncate(0);
    m_baData += content;
    m_eType = SVG;
}

void PixmapSource::setSVG( QString filepath ) {
    m_baData.truncate(0);
    m_path = filepath;
    m_eType = SVG;
}

void PixmapSource::setBitmap( QString filepath ) {
    m_path = filepath;
    m_eType = BITMAP;
}

QString PixmapSource::getId() const {
    quint16 checksum;
    if (m_baData.isEmpty()) {
        checksum = qChecksum( m_path.toAscii().constData(), m_path.length() );
    } else {
        checksum = qChecksum( m_baData.constData(), m_baData.length() );
    }
    return m_path + QString::number(checksum);
}

