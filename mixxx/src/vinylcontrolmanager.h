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

class VinylControlProxy;
class SoundManager;

class VinylControlManager : public QObject, public AudioDestination {
    Q_OBJECT;
  public:
    VinylControlManager(QObject *pParent, ConfigObject<ConfigValue> *pConfig,
            unsigned int nDecks);
    virtual ~VinylControlManager();
    virtual void receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames);
    virtual void onInputConnected(AudioInput input);
    virtual void onInputDisconnected(AudioInput input);
    QList<VinylControlProxy*> vinylControlProxies() const;
    static bool vinylInputEnabled(SoundManager *pSoundManager, int deck);
  public slots:
    void reloadConfig();
  private:
    ConfigObject<ConfigValue> *m_pConfig;
    QList<VinylControlProxy*> m_proxies;
    QSemaphore m_listLock;
};

#endif // VINYLCONTROLMANAGER_H
