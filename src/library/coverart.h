#ifndef COVERART_H
#define COVERART_H

#include <QObject>

#include "trackinfoobject.h"
#include "util/singleton.h"

class CoverArt : public QObject, public Singleton<CoverArt> {
    Q_OBJECT
  public:
    void setConfig(ConfigObject<ConfigValue>* pConfig);

    QString getStoragePath() const;
    QString getDefaultCoverLocation(QString coverArtName);
    QString getDefaultCoverLocation(TrackPointer pTrack);
    QString getDefaultCoverName(QString artist, QString album, QString filename);

    bool deleteFile(const QString& location);
    bool saveFile(QImage cover, QString location);
    QString saveEmbeddedCover(QImage cover, QString artist,
                           QString album, QString filename);
    QString searchCoverArtFile(TrackPointer pTrack);

  protected:
    CoverArt();
    virtual ~CoverArt();

    friend class Singleton<CoverArt>;

  private:
    ConfigObject<ConfigValue>* m_pConfig;

    const char* m_cDefaultImageFormat;

    QString searchInTrackDirectory(QString directory);
};

#endif // COVERART_H
