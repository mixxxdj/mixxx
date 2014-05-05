#ifndef COVERART_H
#define COVERART_H

#include <QObject>

#include "trackinfoobject.h"

class CoverArt : public QObject {
    Q_OBJECT
  public:
    CoverArt(ConfigObject<ConfigValue> *pConfig);
    virtual ~CoverArt();

    QString searchCoverArtFile(TrackInfoObject *pTrack);

  private:
    ConfigObject<ConfigValue>* m_pConfig;

    QString getStoragePath() const;
};

#endif // COVERART_H
