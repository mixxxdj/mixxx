/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"

VinylControlManager::VinylControlManager(QObject *pParent,
        ConfigObject<ConfigValue> *pConfig)
  : QObject(pParent)
  , m_pConfig(pConfig) {

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
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    unsigned char index = input.getIndex();
    QString group = QString("[Channel%1]").arg(index + 1);
    VinylControlProxy *newVC = new VinylControlProxy(m_pConfig, group);
    if (index < m_proxies.size()) {
        if (m_proxies.at(index)) {
            delete m_proxies.at(index);
        }
        m_proxies.replace(index, newVC);
    } else if (index == m_proxies.size()) {
        m_proxies.append(newVC);
    } else { // index > m_proxies.size()
        while (index != m_proxies.size()) m_proxies.append(NULL);
        m_proxies.append(newVC);
    }
}

void VinylControlManager::onInputDisconnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    Q_ASSERT(input.getIndex() < m_proxies.size());
    Q_ASSERT(m_proxies.at(input.getIndex()));

    delete m_proxies.at(input.getIndex());
    m_proxies.replace(input.getIndex(), NULL);
}

