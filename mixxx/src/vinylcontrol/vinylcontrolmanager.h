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
