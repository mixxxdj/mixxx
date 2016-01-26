#ifndef SHOUTCASTMANAGER_H
#define SHOUTCASTMANAGER_H

#include <QObject>

#include "preferences/usersettings.h"
#include "engine/sidechain/engineshoutcast.h"

class SoundManager;

class ShoutcastManager : public QObject {
    Q_OBJECT
  public:
    ShoutcastManager(UserSettingsPointer pConfig,
                     SoundManager* pSoundManager);
    virtual ~ShoutcastManager();

    // Returns true if the Shoutcast connection is enabled. Note this only
    // indicates whether the connection is enabled, not whether it is connected.
    bool isEnabled();

  public slots:
    // Set whether or not the Shoutcast connection is enabled.
    void setEnabled(bool enabled);

  signals:
    void shoutcastEnabled(bool);

  private slots:
    void slotControlEnabled(double v);

  private:
    UserSettingsPointer m_pConfig;
    QSharedPointer<EngineShoutcast> m_pShoutcast;
    ControlObjectSlave* m_pShoutcastEnabled;
};


#endif /* SHOUTCASTMANAGER_H */
