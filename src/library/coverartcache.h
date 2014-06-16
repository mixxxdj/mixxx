#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QFutureWatcher>
#include <QObject>
#include <QPixmapCache>

#include "library/dao/coverartdao.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache>
{
    Q_OBJECT
  public:
    void requestPixmap(QString coverLocation, int trackId);
    void setCoverArtDao(CoverArtDAO* coverdao);
    QString getDefaultCoverLocation(int trackId);

  public slots:
    void imageLoaded();

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);
    void pixmapNotFound(int trackId);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

  private:
    struct coverTuple {
        int trackId;
        QString coverLocation;
        QImage img;
    };

    static CoverArtCache* m_instance;
    CoverArtDAO* m_pCoverArtDAO;
    QSet<int> m_runningIds;

    coverTuple loadImage(QString coverLocation, int trackId);
};

#endif // COVERARTCACHE_H
