/**
 * @file vinylcontrolmanager.h
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#ifndef VINYLCONTROLMANAGER_H
#define VINYLCONTROLMANAGER_H

#include <QObject>
#include <QString>
#include <QTimerEvent>

#include "soundmanagerutil.h"
#include "configobject.h"
#include "vinylcontrol/vinylsignalquality.h"

class ControlObjectSlave;
class ControlObjectThread;
class ControlPushButton;
class SoundManager;
class VinylControl;
class VinylControlProcessor;


const int kMaxNumberOfDecks = 4; // set to 4 because it will ideally not be more
// or less than the number of vinyl-controlled decks but will probably be
// forgotten in any 2->4 deck switchover. Only real consequence is
// sizeof(void*)*2 bytes of wasted memory if we're only using 2 decks -bkgood

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
    VinylControlManager(QObject* pParent, ConfigObject<ConfigValue>* pConfig,
                        SoundManager* pSoundManager);
    virtual ~VinylControlManager();

    // Some initialization must wait until the decks have been created
    void init();

    bool vinylInputEnabled(int deck);
    int vinylInputFromGroup(const QString& group);

    void addSignalQualityListener(VinylSignalQualityListener* pListener);
    void removeSignalQualityListener(VinylSignalQualityListener* pListener);
    void updateSignalQualityListeners();

    void timerEvent(QTimerEvent* pEvent);

  public slots:
    void requestReloadConfig();

  private slots:
    void slotNumDecksChanged(double);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    QSet<VinylSignalQualityListener*> m_listeners;
    VinylControlProcessor* m_pProcessor;
    int m_iTimerId;
    QList<ControlObjectThread*> m_pVcEnabled;
    ControlObjectSlave* m_pNumDecks;
    int m_iNumConfiguredDecks;
};

#endif // VINYLCONTROLMANAGER_H
