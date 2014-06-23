#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QFutureWatcher>
#include <QObject>
#include <QPixmapCache>

#include "library/dao/coverartdao.h"
#include "library/dao/trackdao.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache>
{
    Q_OBJECT
  public:
    void requestPixmap(QString coverLocation, int trackId);
    void setCoverArtDAO(CoverArtDAO* coverdao);
    void setTrackDAO(TrackDAO* trackdao);
    QString getDefaultCoverLocation(int trackId);

  public slots:
    void imageFound();
    void imageLoaded();

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

  private:
    struct FutureResult {
        int trackId;
        QString coverLocation;
        QImage img;
    };

    static CoverArtCache* m_instance;
    CoverArtDAO* m_pCoverArtDAO;
    TrackDAO* m_pTrackDAO;
    QSet<int> m_runningIds;

    FutureResult loadImage(QString coverLocation, int trackId);
    FutureResult searchImage(CoverArtDAO::coverArtInfo coverInfo);
    QImage searchEmbeddedCover(QString trackLocation);
    QString searchInTrackDirectory(QString directory, QString album);
};

#endif // COVERARTCACHE_H
