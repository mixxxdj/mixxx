/**
 * @file vinylcontrolmanager.h
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#ifndef VINYLCONTROLMANAGER_H
#define VINYLCONTROLMANAGER_H

#include <QtCore>

#include "soundmanagerutil.h"
#include "configobject.h"
#include "vinylcontrol/vinylsignalquality.h"

class VinylControlProcessor;
class VinylControl;
class SoundManager;
class ControlPushButton;

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
    VinylControlManager(QObject *pParent, ConfigObject<ConfigValue> *pConfig,
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

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    QSet<VinylSignalQualityListener*> m_listeners;
    VinylControlProcessor* m_pProcessor;
    int m_iTimerId;
};

#endif // VINYLCONTROLMANAGER_H
