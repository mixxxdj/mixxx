#ifndef BROADCAST_BROADCASTMANAGER_H
#define BROADCAST_BROADCASTMANAGER_H

#include <QObject>

#include "engine/sidechain/enginebroadcast.h"
#include "preferences/usersettings.h"

class SoundManager;

class BroadcastManager : public QObject {
    Q_OBJECT
  public:
    BroadcastManager(UserSettingsPointer pConfig,
                     SoundManager* pSoundManager);
    virtual ~BroadcastManager();

    // Returns true if the broadcast connection is enabled. Note this only
    // indicates whether the connection is enabled, not whether it is connected.
    bool isEnabled();

  public slots:
    // Set whether or not the Broadcast connection is enabled.
    void setEnabled(bool enabled);

  signals:
    void broadcastEnabled(bool);

  private slots:
    void slotControlEnabled(double v);

  private:
    UserSettingsPointer m_pConfig;
    QSharedPointer<EngineBroadcast> m_pBroadcast;
    ControlProxy* m_pBroadcastEnabled;
};

#endif /* BROADCAST_BROADCASTMANAGER_H */
