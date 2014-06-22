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

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

  private:
    struct SearchImageResult {
        int trackId;
        QString currentCoverLocation;
        QString coverLocationFound;
        QImage img;
    };

    static CoverArtCache* m_instance;
    CoverArtDAO* m_pCoverArtDAO;
    TrackDAO* m_pTrackDAO;
    QSet<int> m_runningIds;

    SearchImageResult searchImage(CoverArtDAO::coverArtInfo coverInfo);
    bool saveImageOnDisk(QImage cover, QString location);
    QImage searchEmbeddedCover(QString trackLocation);
    QImage searchInTrackDirectory(QString directory, QString album);
};

#endif // COVERARTCACHE_H
