/**
 * @file vinylcontrolmanager.h
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#ifndef VINYLCONTROLMANAGER_H
#define VINYLCONTROLMANAGER_H

#include <QtCore>
#include "soundmanagerutil.h"

class VinylControlProxy;

class VinylControlManager : public QObject, public AudioDestination {
    Q_OBJECT;
  public:
    VinylControlManager(QObject *pParent);
    virtual ~VinylControlManager();
    virtual void receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames);
    virtual void onInputConnected(AudioInput input);
    virtual void onInputDisconnected(AudioInput input);
  private:
    QList<VinylControlProxy*> m_proxies;
};

#endif // VINYLCONTROLMANAGER_H
