#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QFutureWatcher>
#include <QObject>
#include <QPixmapCache>

class CoverArtCache : public QObject
{
    Q_OBJECT
  public:
    static CoverArtCache* getInstance();
    static void destroyInstance();

    void requestPixmap(QString location);

  public slots:
    void imageLoaded();

  signals:
    void responsePixmap(QString location, QPixmap pixmap);

  private:
    CoverArtCache();
    virtual ~CoverArtCache();

    static CoverArtCache* m_instance;
    typedef QPair<QString, QImage> coverPair;
    QStringList m_keys;

    QFutureWatcher<coverPair> m_future_watcher;
    coverPair loadImage(QString location);
};

#endif // COVERARTCACHE_H
