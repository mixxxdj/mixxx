#pragma once

#include <QObject>

#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "engine/sidechain/shoutconnection.h"

class SoundManager;
class ControlPushButton;

class BroadcastManager : public QObject {
    Q_OBJECT
  public:
    enum StatusCOStates {
        STATUSCO_UNCONNECTED = 0, // IDLE state, no error
        STATUSCO_CONNECTING = 1, // 30 s max
        STATUSCO_CONNECTED = 2, // On Air
        STATUSCO_FAILURE = 3, // Happens when all connection fails
        STATUSCO_WARNING = 4
    };

    BroadcastManager(SettingsManager* pSettingsManager,
                     SoundManager* pSoundManager);
    ~BroadcastManager() override;

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
    void slotProfileAdded(BroadcastProfilePtr profile);
    void slotProfileRemoved(BroadcastProfilePtr profile);
    void slotProfilesChanged();
    void slotConnectionStatusChanged(int newState);

  private:
    bool addConnection(BroadcastProfilePtr profile);
    bool removeConnection(BroadcastProfilePtr profile);
    ShoutConnectionPtr findConnectionForProfile(BroadcastProfilePtr profile);

    UserSettingsPointer m_pConfig;
    BroadcastSettingsPointer m_pBroadcastSettings;
    QSharedPointer<EngineNetworkStream> m_pNetworkStream;

    ControlPushButton* m_pBroadcastEnabled;
    ControlObject* m_pStatusCO;
};
