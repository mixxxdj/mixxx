#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QObject>

#include "library/coverart.h"
#include "library/dao/coverartdao.h"
#include "library/dao/trackdao.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache> {
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
     * @param onlyCached : if it is 'true', the method will NOT try to load
     *      covers from the given 'coverLocation' and it will also NOT run the
     *      search algorithm.
     *      In this way, the method will just look into CoverCache and return
     *      a Pixmap if it is already loaded in the QPixmapCache.
     */
    QPixmap requestPixmap(const CoverInfo& info,
                          const QSize& croppedSize = QSize(0,0),
                          const bool onlyCached = false,
                          const bool issueRepaint = false);

    bool changeCoverArt(int trackId, const QString& newCoverLocation="");
    void setCoverArtDAO(CoverArtDAO* coverdao);
    void setTrackDAO(TrackDAO* trackdao);

    // This is for widgets that try to get the covers directly from the cache
    // instead of waiting for the signal with the cover. Because we update the
    // database in large batches it can happen that a widget looks up a track
    // that we couldn't save yet. Don't use this unless you really need to. This
    // method will look in the hash we keep with information to save in the
    // database and return the coverlocation if it is in there. Otherweise an
    // empty string is returned.
    QString trackInDBHash(int trackId);

    struct FutureResult {
        int trackId;
        QString coverLocation;
        QString hash;
        QImage img;
        QSize croppedSize;
        bool issueRepaint;
    };

    FutureResult searchImage(CoverArtDAO::CoverArtInfo coverInfo,
                             const QSize& croppedSize,
                             const bool emitSignals);

  public slots:
    void imageFound();
    void imageLoaded();

  private slots:
    void updateDB(bool forceUpdate=false);

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);
    void requestRepaint();

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    FutureResult loadImage(CoverArtDAO::CoverArtInfo coverInfo,
                           const QSize &croppedSize,
                           const bool emitSignals);

  private:
    CoverArtDAO* m_pCoverArtDAO;
    TrackDAO* m_pTrackDAO;
    QSet<int> m_runningIds;
    QHash<int, QPair<QString, QString> > m_queueOfUpdates;
};

#endif // COVERARTCACHE_H
