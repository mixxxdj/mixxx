/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"
#include "vinylcontrolxwax.h"
#include "soundmanager.h"

const int kNumberOfDecks = 4; // set to 4 because it will ideally not be more
// or less than the number of vinyl-controlled decks but will probably be
// forgotten in any 2->4 deck switchover. Only real consequence is
// sizeof(void*)*2 bytes of wasted memory if we're only using 2 decks -bkgood

const QString kVCProxyGroup = QString("[Channel%1]");

VinylControlManager::VinylControlManager(QObject *pParent,
        ConfigObject<ConfigValue> *pConfig)
  : QObject(pParent)
  , m_pConfig(pConfig)
  , m_proxies(kNumberOfDecks, NULL) {
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
    m_proxiesLock.lockForWrite();
    for (int i = 0; i < m_proxies.size(); ++i) {
        if (m_proxies.at(i)) {
            delete m_proxies.at(i);
            m_proxies.replace(i, NULL);
        }
    }
    m_proxiesLock.unlock();

    // xwax has a global LUT that we need to free after we've shut down our
    // vinyl control threads because it's not thread-safe.
    VinylControlXwax::freeLUTs();

    // save a bunch of stuff to config
    // turn off vinyl control so it won't be enabled on load (this is redundant to mixxx.cpp)
    m_pConfig->set(ConfigKey("[VinylControl]","enabled_ch1"), false);
    m_pConfig->set(ConfigKey("[VinylControl]","enabled_ch2"), false);
    m_pConfig->set(ConfigKey("[Channel 1]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[Channel 2]","vinylcontrol_enabled"), false);
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
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    if (m_proxiesLock.tryLockForRead()) {
        Q_ASSERT(input.getIndex() < m_proxies.size());
        VinylControlProxy *pProxy(m_proxies.at(input.getIndex()));
        Q_ASSERT(pProxy);
        pProxy->AnalyseSamples(pBuffer, nFrames);
        m_proxiesLock.unlock();
    }
}

void VinylControlManager::onInputConnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    unsigned char index = input.getIndex();
    VinylControlProxy *pNewVC = new VinylControlProxy(m_pConfig,
            kVCProxyGroup.arg(index + 1));
    m_proxiesLock.lockForWrite();
    if (index < m_proxies.size()) {
        if (m_proxies.at(index)) {
            delete m_proxies.at(index);
        }
        m_proxies.replace(index, pNewVC);
    } else {
        m_proxies.resize(index + 1);
        m_proxies.replace(index, pNewVC);
    }
    m_proxiesLock.unlock();
}

void VinylControlManager::onInputDisconnected(AudioInput input) {
    Q_ASSERT(input.getType() == AudioInput::VINYLCONTROL);
    m_proxiesLock.lockForWrite();
    Q_ASSERT(input.getIndex() < m_proxies.size());
    Q_ASSERT(m_proxies.at(input.getIndex()));
    
    delete m_proxies.at(input.getIndex());
    m_proxies.replace(input.getIndex(), NULL);
    m_proxiesLock.unlock();
}

void VinylControlManager::reloadConfig() {
    m_proxiesLock.lockForWrite();
    for (int i = 0; i < m_proxies.size(); ++i) {
        if (!m_proxies.at(i)) continue;
        VinylControlProxy *pProxy = m_proxies.at(i);
        QString group = kVCProxyGroup.arg(i + 1);
        delete pProxy;
        pProxy = new VinylControlProxy(m_pConfig, group);
        m_proxies.replace(i, pProxy);
    }
    m_proxiesLock.unlock();
}

QList<VinylControlProxy*> VinylControlManager::vinylControlProxies() {
    m_proxiesLock.lockForRead();
    QList<VinylControlProxy*> list(m_proxies.toList());
    m_proxiesLock.unlock();
    return list;
}

bool VinylControlManager::vinylInputEnabled(int deck) {
    // a vinylcontrolproxy is only created if vinyl control is enabled for
    // a deck, so...
    m_proxiesLock.lockForRead();
    bool ret = (deck - 1) < m_proxies.size() && m_proxies[deck-1];
    m_proxiesLock.unlock();
    return ret;
}
