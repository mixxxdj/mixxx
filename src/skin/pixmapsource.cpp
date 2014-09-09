#include <QtDebug>

#include "skin/pixmapsource.h"

PixmapSource::PixmapSource() {
}

PixmapSource::~PixmapSource() {
}

QByteArray PixmapSource::getData() const {
    return data;
}

QString PixmapSource::getType() const {
    return type;
}

QString PixmapSource::getPath() const {
    return path;
}

void PixmapSource::setPath( QString newPath ) {
    path = newPath;
}

bool PixmapSource::isEmpty() {
    return path.isEmpty() && data.isEmpty() ;
}

void PixmapSource::setSVG( QByteArray content ) {
    data.truncate(0);
    data += content;
    type = "svg";
}

void PixmapSource::setSVG( QString filepath ) {
    data.truncate(0);
    path = filepath;
    type = "svg";
}

void PixmapSource::setBitmap( QString filepath ) {
    path = filepath;
    type = "bitmap";
}

QString PixmapSource::getId() const {
    quint16 checksum;
    QString out;
    if (data.isEmpty()) {
        checksum = qChecksum( path.toAscii().constData(), path.length() );
    } else {
        checksum = qChecksum( data.constData(), data.length() );
    }
    return path + out.setNum(checksum);
}

