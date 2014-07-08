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
    void requestPixmap(int trackId,
                       QPixmap& pixmap,
                       const QString& coverLocation = QString(),
                       const QString& md5Hash = QString());
    void setCoverArtDAO(CoverArtDAO* coverdao);
    void setTrackDAO(TrackDAO* trackdao);
    QString getDefaultCoverLocation(int trackId);
    QPixmap getDefaultCoverArt() { return m_defaultCover; }

  public slots:
    void imageFound();
    void imageLoaded();

  signals:
    void pixmapFound(int trackId);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    struct FutureResult {
        int trackId;
        QString coverLocation;
        QString md5Hash;
        QImage img;
    };

    FutureResult searchImage(CoverArtDAO::CoverArtInfo coverInfo);
    FutureResult loadImage(int trackId,
                           const QString& coverLocation,
                           const QString& md5Hash);

  private:
    static CoverArtCache* m_instance;
    CoverArtDAO* m_pCoverArtDAO;
    TrackDAO* m_pTrackDAO;
    QSet<int> m_runningIds;
    QPixmap* m_pixmap;
    const QPixmap m_defaultCover;

    QString calculateMD5(QImage img);
    QImage rescaleBigImage(QImage img);
    QImage searchEmbeddedCover(QString trackLocation);
    QString searchInTrackDirectory(QString directory,
                                   QString trackBaseName,
                                   QString album);
};

#endif // COVERARTCACHE_H
