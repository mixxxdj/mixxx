#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QFutureWatcher>
#include <QObject>
#include <QPixmapCache>

#include "trackinfoobject.h"

class CoverArtCache : public QObject
{
    Q_OBJECT
  public:
    static CoverArtCache* getInstance();
    static void destroyInstance();

    void requestPixmap(TrackPointer pTrack);

  public slots:
    void imageLoaded();

  signals:
    void pixmapFound(QString location, QPixmap pixmap);
    void pixmapNotFound(TrackPointer);

  private:
    CoverArtCache();
    virtual ~CoverArtCache();

    static CoverArtCache* m_instance;
    typedef QPair<TrackPointer, QImage> coverPair;
    QStringList m_runningLocations;

    coverPair loadImage(TrackPointer pTrack);
};

#endif // COVERARTCACHE_H
