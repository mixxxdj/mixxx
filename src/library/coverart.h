#ifndef COVERART_H
#define COVERART_H

#include <QImage>
#include <QString>
#include <QObject>

struct CoverInfo {
    CoverInfo() : trackId(-1),
                  coverLocation(QString()),
                  trackLocation(QString()),
                  hash(QString()) {}

    int trackId;
    QString coverLocation;
    QString trackLocation;
    QString hash;
};

struct CoverAndAlbumInfo {
    CoverAndAlbumInfo() {}

    CoverInfo info;
    QString album;
};

struct CoverArt {
    CoverArt() {}

    CoverInfo info;
    QImage image;
};

#endif /* COVERART_H */
