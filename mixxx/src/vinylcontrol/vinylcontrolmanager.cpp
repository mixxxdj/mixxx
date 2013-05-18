/**
 * @file vinylcontrolmanager.cpp
 * @author Bill Good <bkgood@gmail.com>
 * @date April 15, 2011
 */

#include "vinylcontrolmanager.h"
#include "vinylcontrolproxy.h"
#include "vinylcontrolxwax.h"
#include "soundmanager.h"
#include "controlpushbutton.h"
#include "util/timer.h"

const int kNumberOfDecks = 4; // set to 4 because it will ideally not be more
// or less than the number of vinyl-controlled decks but will probably be
// forgotten in any 2->4 deck switchover. Only real consequence is
// sizeof(void*)*2 bytes of wasted memory if we're only using 2 decks -bkgood

const QString kVCProxyGroup = QString("[Channel%1]");

VinylControlManager::VinylControlManager(QObject *pParent,
        ConfigObject<ConfigValue> *pConfig)
  : QObject(pParent)
  , m_pConfig(pConfig)
  , m_proxies(kNumberOfDecks, NULL)
  , m_pToggle(new ControlPushButton(ConfigKey("[VinylControl]", "Toggle")))
{
    connect(m_pToggle, SIGNAL(valueChanged(double)), SLOT(toggleDeck(double)));
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
    m_pConfig->set(ConfigKey("[Channel 1]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[Channel 2]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[Channel 3]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[Channel 4]","vinylcontrol_enabled"), false);
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch1"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel1]","vinylcontrol_cueing"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch2"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel2]","vinylcontrol_cueing"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch3"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel3]","vinylcontrol_cueing"))->get()));
    m_pConfig->set(ConfigKey("[VinylControl]","cueing_ch4"),
        ConfigValue((int)ControlObject::getControl(
            ConfigKey("[Channel4]","vinylcontrol_cueing"))->get()));
    delete m_pToggle;
}

void VinylControlManager::init()
{
    // Load saved preferences now that the objects exist
    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_enabled"))
            ->setValueFromThread(0, NULL);
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_enabled"))
            ->setValueFromThread(0, NULL);
    ControlObject::getControl(ConfigKey("[Channel3]","vinylcontrol_enabled"))
            ->setValueFromThread(0, NULL);
    ControlObject::getControl(ConfigKey("[Channel4]","vinylcontrol_enabled"))
            ->setValueFromThread(0, NULL);

    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_mode"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_mode"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel3]","vinylcontrol_mode"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel4]","vinylcontrol_mode"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","mode")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel1]","vinylcontrol_cueing"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch1")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel2]","vinylcontrol_cueing"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch2")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel3]","vinylcontrol_cueing"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch3")).toDouble(), NULL);
    ControlObject::getControl(ConfigKey("[Channel4]","vinylcontrol_cueing"))
            ->setValueFromThread(m_pConfig->getValueString(
                ConfigKey("[VinylControl]","cueing_ch4")).toDouble(), NULL);
}

void VinylControlManager::receiveBuffer(AudioInput input,
        const short *pBuffer, unsigned int nFrames) {
    ScopedTimer t("VinylControlManager::receiveBuffer");
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring incoming buffer.";
        return;
    }
    if (m_proxiesLock.tryLockForRead()) {
        if (input.getIndex() >= m_proxies.size()) {
            qDebug() << "WARNING: AudioInput index out of bounds. Ignoring incoming buffer.";
            return;
        }
        VinylControlProxy* pProxy = m_proxies.at(input.getIndex());
        if (pProxy == NULL) {
            qDebug() << "WARNING: AudioInput index proxy does not exist. Ignoring incoming buffer.";
            return;
        }
        pProxy->AnalyseSamples(pBuffer, nFrames);
        m_proxiesLock.unlock();
    }
}

void VinylControlManager::onInputConnected(AudioInput input) {
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }
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
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }
    m_proxiesLock.lockForWrite();
    if (input.getIndex() >= m_proxies.size()) {
        qDebug() << "AudioInput index out of bounds. Ignoring.";
        return;
    }
    VinylControlProxy* pVC = m_proxies.at(input.getIndex());
    m_proxies.replace(input.getIndex(), NULL);
    delete pVC;
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

VinylControlProxy* VinylControlManager::getVinylControlProxyForChannel(QString channel)
{
    // TODO: will need update for n-deck
    if (channel == "[Channel1]") {
        return m_proxies.at(0);
    } else if (channel == "[Channel2]") {
        return m_proxies.at(1);
    } else if (channel == "[Channel3]") {
        return m_proxies.at(2);
    } else if (channel == "[Channel4]") {
        return m_proxies.at(3);
    }

    return NULL;
}

void VinylControlManager::toggleDeck(double value) {
    if (!value) return;
    /** few different cases here:
     * 1. No decks have vinyl control enabled.
     * 2. One deck has vinyl control enabled.
     * 3. Many decks have vinyl control enabled.
     *
     * For case 1, we'll just enable vinyl control on the first deck. Case 2
     * is the most common one, we'll just turn off the vinyl control on the
     * deck currently using it and turn it on on the next one (sequentially,
     * wrapping as needed). Behaviour in case 3 is totally non-obvious and
     * will be ignored.
     */
    m_proxiesLock.lockForRead();
    int enabled(-1); // -1 means we haven't found a proxy that's enabled
    for (int i = 0; i < m_proxies.size(); ++i) {
        if (m_proxies[i] && m_proxies[i]->isEnabled()) {
            if (enabled > -1) goto bail; // case 3
            enabled = i;
        }
    }
    if (enabled > -1 && m_proxies.size() > 1) {
        // handle case 2
        int nextProxy((enabled + 1) % m_proxies.size());
        while (!m_proxies[nextProxy]) {
            nextProxy = (nextProxy + 1) % m_proxies.size();
        } // guaranteed to terminate as there's at least 1 non-null proxy
        if (nextProxy == enabled) goto bail;
        m_proxies[enabled]->ToggleVinylControl(false);
        m_proxies[nextProxy]->ToggleVinylControl(true);
    } else if (enabled == -1) {
        // handle case 1, or we just don't have any proxies
        foreach (VinylControlProxy *proxy, m_proxies) {
            if (proxy) {
                proxy->ToggleVinylControl(true);
                break;
            }
        }
    }
bail:
    m_proxiesLock.unlock();
}
