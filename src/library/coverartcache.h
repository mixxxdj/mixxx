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
    /* This method is used to request a cover art pixmap.
     *
     * @param croppedSize : QSize(finalCoverWidth, finalCoverHeight)
     *      it determines the final cover size.
     *      Use QSize() to get the original size.
     *      NOTE!
     *          the cover will be resized to 'finalCoverWidth' and
     *          it'll be cropped from the top until the finalCoverHeight' pixel
     *
     * @param tryLoadAndSearch : if it is 'true', the method will NOT try to load
     *      covers from the given 'coverLocation' and it will also NOT run the
     *      search algorithm.
     *      In this way, the method will just look into CoverCache and return
     *      a Pixmap if it is already loaded in the QPixmapCache.
     */
    QPixmap requestPixmap(int trackId,
                          const QString& coverLocation = QString(),
                          const QString& md5Hash = QString(),
                          const QSize& croppedSize = QSize(0,0),
                          const bool tryLoadAndSearch = true,
                          const bool issueRepaint = false);

    bool changeCoverArt(int trackId, const QString& newCoverLocation="");
    void setCoverArtDAO(CoverArtDAO* coverdao);
    void setTrackDAO(TrackDAO* trackdao);
    QString getDefaultCoverLocation() { return m_sDefaultCoverLocation; }
    QPixmap getDefaultCoverArt() { return m_defaultCover; }
    TrackPointer getTrack(int trackId);

  public slots:
    void imageFound();
    void imageLoaded();

  private slots:
    void updateDB();

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);
    void requestRepaint();

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    struct FutureResult {
        int trackId;
        QString coverLocation;
        QString md5Hash;
        QImage img;
        QSize croppedSize;
        bool issueRepaint;
        bool newImgFound;
    };

    FutureResult searchImage(CoverArtDAO::CoverArtInfo coverInfo,
                             const QSize& croppedSize,
                             const bool emitSignals);
    FutureResult loadImage(int trackId,
                           const QString& coverLocation,
                           const QString& md5Hash,
                           const QSize &croppedSize,
                           const bool emitSignals);

  private:
    static CoverArtCache* m_instance;
    CoverArtDAO* m_pCoverArtDAO;
    TrackDAO* m_pTrackDAO;
    const QString m_sDefaultCoverLocation;
    const QPixmap m_defaultCover;
    QTimer* m_timer;
    QSet<int> m_runningIds;
    QHash<int, QPair<QString, QString> > m_queueOfUpdates;

    // @param img: image that will be cropped
    // @param size: (desired cover width, cell height)
    QImage cropImage(QImage img, const QSize& finalSize);

    QString calculateMD5(QImage img);
    QImage rescaleBigImage(QImage img);
    QImage extractEmbeddedCover(QString trackLocation);
    QString searchInTrackDirectory(QString directory,
                                   QString trackBaseName,
                                   QString album);
};

#endif // COVERARTCACHE_H
