#pragma once

#include <QObject>
#include <QString>
#include <QTimerEvent>

#include "preferences/usersettings.h"
#include "soundio/soundmanagerutil.h"
#include "util/defs.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlProxy;
class ControlPushButton;
class SoundManager;
class VinylControl;
class VinylControlProcessor;


// VinylControlManager is the main-thread interface that other parts of Mixxx
// use to interact with the vinyl control subsystem (other than controls exposed
// by vinyl control to the rest of Mixxx). VinylControlManager starts a
// VinylControlProcessor thread which is in charge of receiving samples from the
// engine and processing them. The separation of VinylControlManager and
// VinylControlProcessor allows us to keep a more clear separation between the
// main thread, the VC thread, and the engine callback.
class VinylControlManager : public QObject {
    Q_OBJECT;
  public:
    VinylControlManager(QObject* pParent, UserSettingsPointer pConfig,
                        SoundManager* pSoundManager);
    virtual ~VinylControlManager();

    // Some initialization must wait until the decks have been created
    void init();

    bool vinylInputConnected(int deck);
    int vinylInputFromGroup(const QString& group);

    void addSignalQualityListener(VinylSignalQualityListener* pListener);
    void removeSignalQualityListener(VinylSignalQualityListener* pListener);
    void updateSignalQualityListeners();

    void timerEvent(QTimerEvent* pEvent);

  public slots:
    void requestReloadConfig();
    void toggleVinylControl(int deck);

  signals:
    void vinylControlDeckEnabled(int deck, bool enabled);

  private slots:
    void slotNumDecksChanged(double);
    void slotVinylControlEnabledChanged(int deck);

  private:
    UserSettingsPointer m_pConfig;
    QSet<VinylSignalQualityListener*> m_listeners;
    VinylControlProcessor* m_pProcessor;
    int m_iTimerId;
    QList<ControlProxy*> m_pVcEnabled;
    ControlProxy* m_pNumDecks;
    int m_iNumConfiguredDecks;
};
