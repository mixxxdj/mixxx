/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"

VinylControlManager::VinylControlManager(QObject *pParent,
        ConfigObject<ConfigValue> *pConfig, unsigned int nDecks)
  : QObject(pParent)
  , m_pConfig(pConfig)
  , m_listLock(nDecks) {
    while (m_proxies.size() < static_cast<int>(nDecks)) m_proxies.append(NULL);
}

VinylControlManager::~VinylControlManager() {
    m_listLock.acquire(m_proxies.size());
    while (m_proxies.size() > 0) {
        VinylControlProxy *pProxy = m_proxies.takeFirst();
        delete pProxy;
    }
    m_listLock.release(m_proxies.size());
}

void VinylControlManager::receiveBuffer(AudioInput input,
        const short *pBuffer, unsigned int nFrames) {
    Q_ASSERT(input.getIndex() < m_proxies.size());
    if (m_listLock.tryAcquire(1)) {
        VinylControlProxy *pProxy(m_proxies.at(input.getIndex()));
        Q_ASSERT(pProxy);
        pProxy->AnalyseSamples(pBuffer, nFrames);
        m_listLock.release(1);
    }
}

void VinylControlManager::onInputConnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    unsigned char index = input.getIndex();
    Q_ASSERT(index < m_proxies.size());
    VinylControlProxy *pNewVC = new VinylControlProxy(m_pConfig,
            QString("[Channel%1]").arg(index + 1));
    m_listLock.acquire(m_proxies.size());
    if (m_proxies.at(index)) {
        delete m_proxies.at(index);
    }
    m_proxies.replace(index, pNewVC);
    m_listLock.release(m_proxies.size());
}

void VinylControlManager::onInputDisconnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    Q_ASSERT(input.getIndex() < m_proxies.size());
    m_listLock.acquire(m_proxies.size());
    Q_ASSERT(m_proxies.at(input.getIndex()));

    delete m_proxies.at(input.getIndex());
    m_proxies.replace(input.getIndex(), NULL);
    m_listLock.release(m_proxies.size());
}

void VinylControlManager::reloadConfig() {
    m_listLock.acquire(m_proxies.size());

    for (int i = 0; i < m_proxies.size(); ++i) {
        if (!m_proxies.at(i)) continue;
        VinylControlProxy *pProxy = m_proxies.at(i);
        QString group = QString("[Channel%1]").arg(i + 1);
        delete pProxy;
        pProxy = new VinylControlProxy(m_pConfig, group);
        m_proxies.replace(i, pProxy);
    }
    m_listLock.release(m_proxies.size());
}
