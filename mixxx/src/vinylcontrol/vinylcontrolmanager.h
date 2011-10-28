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
class ControlPushButton;

class VinylControlManager : public QObject, public AudioDestination {
    Q_OBJECT;
  public:
    VinylControlManager(QObject *pParent, ConfigObject<ConfigValue> *pConfig);
    virtual ~VinylControlManager();
    virtual void receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames);
    virtual void onInputConnected(AudioInput input);
    virtual void onInputDisconnected(AudioInput input);
    QList<VinylControlProxy*> vinylControlProxies();
    bool vinylInputEnabled(int deck);
    VinylControlProxy* getVinylControlProxyForChannel(QString);
  public slots:
    void reloadConfig();
  signals:
  private slots:
    void toggleDeck(double value);
  private:
    ConfigObject<ConfigValue> *m_pConfig;
    QVector<VinylControlProxy*> m_proxies;
    QReadWriteLock m_proxiesLock;
    ControlPushButton *m_pToggle;
};

#endif // VINYLCONTROLMANAGER_H
