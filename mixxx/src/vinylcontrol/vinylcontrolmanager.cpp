/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"
#include "vinylcontrolxwax.h"
#include "soundmanager.h"

VinylControlManager::VinylControlManager(QObject *pParent,
        ConfigObject<ConfigValue> *pConfig, unsigned int nDecks)
  : QObject(pParent)
  , m_pConfig(pConfig)
  , m_proxies(nDecks, NULL)
  , m_proxiesLock(nDecks) {
    // load a bunch of stuff
    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_enabled"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","enabled_ch1")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_enabled"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","enabled_ch2")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_mode"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_mode"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_cueing"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch1")).toDouble());
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_cueing"))
        ->queueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch2")).toDouble());
}

VinylControlManager::~VinylControlManager() {
    m_proxiesLock.acquire(m_proxies.size());
    for (int i = 0; i < m_proxies.size(); ++i) {
        if (m_proxies.at(i)) {
            delete m_proxies.at(i);
            m_proxies.replace(i, NULL);
        }
    }
    m_proxiesLock.release(m_proxies.size());

    // xwax has a global LUT that we need to free after we've shut down our
    // vinyl control threads because it's not thread-safe.
    VinylControlXwax::freeLUTs();

    // save a bunch of stuff to config
    m_pConfig->set(ConfigKey("[VinylControl]","enabled_ch1"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel1]","vinylcontrol_enabled"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","enabled_ch2"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel2]","vinylcontrol_enabled"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","mode"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel1]","vinylcontrol_mode"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","mode"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel2]","vinylcontrol_mode"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch1"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel1]","vinylcontrol_cueing"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch2"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel2]","vinylcontrol_cueing"))->get()));
}

void VinylControlManager::receiveBuffer(AudioInput input,
        const short *pBuffer, unsigned int nFrames) {
    Q_ASSERT(input.getIndex() < m_proxies.size());
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    if (m_proxiesLock.tryAcquire(1)) {
        VinylControlProxy *pProxy(m_proxies.at(input.getIndex()));
        Q_ASSERT(pProxy);
        pProxy->AnalyseSamples(pBuffer, nFrames);
        m_proxiesLock.release(1);
    }
}

void VinylControlManager::onInputConnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    unsigned char index = input.getIndex();
    Q_ASSERT(index < m_proxies.size());
    VinylControlProxy *pNewVC = new VinylControlProxy(m_pConfig,
            QString("[Channel%1]").arg(index + 1));
    m_proxiesLock.acquire(m_proxies.size());
    if (m_proxies.at(index)) {
        delete m_proxies.at(index);
    }
    m_proxies.replace(index, pNewVC);
    m_proxiesLock.release(m_proxies.size());
}

void VinylControlManager::onInputDisconnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    Q_ASSERT(input.getIndex() < m_proxies.size());
    m_proxiesLock.acquire(m_proxies.size());
    Q_ASSERT(m_proxies.at(input.getIndex()));

    delete m_proxies.at(input.getIndex());
    m_proxies.replace(input.getIndex(), NULL);
    m_proxiesLock.release(m_proxies.size());
}

void VinylControlManager::reloadConfig() {
    m_proxiesLock.acquire(m_proxies.size());

    for (int i = 0; i < m_proxies.size(); ++i) {
        if (!m_proxies.at(i)) continue;
        VinylControlProxy *pProxy = m_proxies.at(i);
        QString group = QString("[Channel%1]").arg(i + 1);
        delete pProxy;
        pProxy = new VinylControlProxy(m_pConfig, group);
        m_proxies.replace(i, pProxy);
    }
    m_proxiesLock.release(m_proxies.size());
}

QList<VinylControlProxy*> VinylControlManager::vinylControlProxies() {
    m_proxiesLock.acquire(m_proxies.size());
    QList<VinylControlProxy*> list(m_proxies.toList());
    m_proxiesLock.release(m_proxies.size());
    return list;
}

bool VinylControlManager::vinylInputEnabled(int deck) {
    // a vinylcontrolproxy is only created if vinyl control is enabled for
    // a deck, so...
    return (deck - 1) < m_proxies.size() && m_proxies[deck-1];
}
