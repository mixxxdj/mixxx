#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QFutureWatcher>
#include <QObject>
#include <QPixmapCache>

#include "trackinfoobject.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache>
{
    Q_OBJECT
  public:
    void requestPixmap(TrackPointer pTrack);

  public slots:
    void imageLoaded();

  signals:
    void pixmapFound(QString location, QPixmap pixmap);
    void pixmapNotFound(TrackPointer);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

  private:
    static CoverArtCache* m_instance;
    typedef QPair<TrackPointer, QImage> coverPair;
    QStringList m_runningLocations;

    coverPair loadImage(TrackPointer pTrack);
};

#endif // COVERARTCACHE_H
