/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"

VinylControlManager::VinylControlManager(QObject *pParent)
  : QObject(pParent) {

}

VinylControlManager::~VinylControlManager() {
    while (m_proxies.size() > 0) {
        VinylControlProxy *pProxy = m_proxies.takeFirst();
        delete pProxy;
    }
}

void VinylControlManager::receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames) {

}

void VinylControlManager::onInputConnected(AudioInput input) {

}

void VinylControlManager::onInputDisconnected(AudioInput input) {

}

